#pragma once

#include <Parsers/IAST.h>
#include <Parsers/ASTQueryWithOutput.h>
// #include <Parsers/ASTQueryWithOnCluster.h>

namespace DB
{


/** DROP query
  */
class ASTDropQuery : public ASTQueryWithOutput// , public ASTQueryWithOnCluster
{
public:
    bool detach{false};    /// DETACH query, not DROP.
    bool if_exists{false};
    String database;
    String table;

    ASTDropQuery() = default;
    explicit ASTDropQuery(const StringRange range_) : ASTQueryWithOutput(range_) {}

    /** Get the text that identifies this element. */
    String getID() const override { return (detach ? "DetachQuery_" : "DropQuery_") + database + "_" + table; };

    ASTPtr clone() const override
    {
        auto res = std::make_shared<ASTDropQuery>(*this);
        cloneOutputOptions(*res);
        return res;
    }

protected:
    void formatQueryImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override
    {
        if (table.empty() && !database.empty())
        {
            settings.ostr << (settings.hilite ? hilite_keyword : "")
                << (detach ? "DETACH DATABASE " : "DROP DATABASE ")
                << (if_exists ? "IF EXISTS " : "")
                << (settings.hilite ? hilite_none : "")
                << backQuoteIfNeed(database);
        }
        else
        {
            settings.ostr << (settings.hilite ? hilite_keyword : "")
                << (detach ? "DETACH TABLE " : "DROP TABLE ")
                << (if_exists ? "IF EXISTS " : "") << (settings.hilite ? hilite_none : "")
                << (!database.empty() ? backQuoteIfNeed(database) + "." : "") << backQuoteIfNeed(table);
        }
    }
};

}
