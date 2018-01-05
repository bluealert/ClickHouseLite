#pragma once

#include <Parsers/IAST.h>
#include <Parsers/ASTQueryWithOutput.h>
// #include <Parsers/ASTQueryWithOnCluster.h>

namespace DB
{


/** RENAME query
  */
class ASTRenameQuery : public ASTQueryWithOutput// , public ASTQueryWithOnCluster
{
public:
    struct Table
    {
        String database;
        String table;
    };

    struct Element
    {
        Table from;
        Table to;
    };

    using Elements = std::vector<Element>;
    Elements elements;

    ASTRenameQuery() = default;
    explicit ASTRenameQuery(const StringRange range_) : ASTQueryWithOutput(range_) {}

    /** Get the text that identifies this element. */
    String getID() const override { return "Rename"; };

    ASTPtr clone() const override
    {
        auto res = std::make_shared<ASTRenameQuery>(*this);
        cloneOutputOptions(*res);
        return res;
    }

protected:
    void formatQueryImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override
    {
        settings.ostr << (settings.hilite ? hilite_keyword : "") << "RENAME TABLE " << (settings.hilite ? hilite_none : "");

        for (auto it = elements.cbegin(); it != elements.cend(); ++it)
        {
            if (it != elements.cbegin())
                settings.ostr << ", ";

            settings.ostr << (!it->from.database.empty() ? backQuoteIfNeed(it->from.database) + "." : "") << backQuoteIfNeed(it->from.table)
                << (settings.hilite ? hilite_keyword : "") << " TO " << (settings.hilite ? hilite_none : "")
                << (!it->to.database.empty() ? backQuoteIfNeed(it->to.database) + "." : "") << backQuoteIfNeed(it->to.table);
        }
    }
};

}
