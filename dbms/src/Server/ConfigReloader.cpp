#include "ConfigReloader.h"

#include <Poco/Util/Application.h>
#include <Poco/File.h>

#include <common/logger_useful.h>

#include <Interpreters/Context.h>
#include <Common/setThreadName.h>
#include <Common/ConfigProcessor.h>


namespace DB
{

constexpr decltype(ConfigReloader::reload_interval) ConfigReloader::reload_interval;

ConfigReloader::ConfigReloader(const std::string & path_, const std::string & include_from_path_,
        Updater && updater_, bool already_loaded)
    : path(path_), include_from_path(include_from_path_) , updater(std::move(updater_))
{
    if (!already_loaded)
        reloadIfNewer(/* force = */ true, /* throw_on_error = */ true, /* fallback_to_preprocessed = */ true);

    thread = std::thread(&ConfigReloader::run, this);
}


ConfigReloader::~ConfigReloader()
{
    try
    {
        quit = true;
        thread.join();
    }
    catch (...)
    {
        DB::tryLogCurrentException(__PRETTY_FUNCTION__);
    }
}


void ConfigReloader::run()
{
    setThreadName("ConfigReloader");
}

void ConfigReloader::reloadIfNewer(bool force, bool throw_on_error, bool fallback_to_preprocessed)
{
    FilesChangesTracker new_files = getNewFileList();
    if (force || new_files.isDifferOrNewerThan(files))
    {
        ConfigProcessor::LoadedConfig loaded_config;
        try
        {
            LOG_DEBUG(log, "Loading config `" << path << "'");

            loaded_config = ConfigProcessor().loadConfig(path, /* allow_zk_includes = */ true);
        }
        catch (...)
        {
            if (throw_on_error)
                throw;

            tryLogCurrentException(log, "Error loading config from `" + path + "'");
            return;
        }

        /** We should remember last modification time if and only if config was sucessfully loaded
         * Otherwise a race condition could occur during config files update:
         *  File is contain raw (and non-valid) data, therefore config is not applied.
         *  When file has been written (and contain valid data), we don't load new data since modification time remains the same.
         */
        if (!loaded_config.loaded_from_preprocessed)
            files = std::move(new_files);

        try
        {
            updater(loaded_config.configuration);
        }
        catch (...)
        {
            if (throw_on_error)
                throw;
            tryLogCurrentException(log, "Error updating configuration from `" + path + "' config.");
        }
    }
}

struct ConfigReloader::FileWithTimestamp
{
    std::string path;
    time_t modification_time;

    FileWithTimestamp(std::string path_, time_t modification_time_)
        : path(std::move(path_)), modification_time(modification_time_) {}

    bool operator < (const FileWithTimestamp & rhs) const
    {
        return path < rhs.path;
    }

    static bool isTheSame(const FileWithTimestamp & lhs, const FileWithTimestamp & rhs)
    {
        return (lhs.modification_time == rhs.modification_time) && (lhs.path == rhs.path);
    }
};


void ConfigReloader::FilesChangesTracker::addIfExists(const std::string & path)
{
    if (!path.empty() && Poco::File(path).exists())
    {
        files.emplace(path, Poco::File(path).getLastModified().epochTime());
    }
}

bool ConfigReloader::FilesChangesTracker::isDifferOrNewerThan(const FilesChangesTracker & rhs)
{
    return (files.size() != rhs.files.size()) ||
        !std::equal(files.begin(), files.end(), rhs.files.begin(), FileWithTimestamp::isTheSame);
}

ConfigReloader::FilesChangesTracker ConfigReloader::getNewFileList() const
{
    FilesChangesTracker file_list;

    file_list.addIfExists(path);
    file_list.addIfExists(include_from_path);

    for (const auto & merge_path : ConfigProcessor::getConfigMergeFiles(path))
        file_list.addIfExists(merge_path);

    return file_list;
}

}
