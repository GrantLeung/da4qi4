#include "router.hpp"

namespace da4qi4
{

router_equals operator "" _router_equals(char const* str, size_t n)
{
    return router_equals(std::string(str, n));
}

router_starts operator "" _router_starts(char const* str, size_t n)
{
    return router_starts(std::string(str, n));
}

router_regex operator "" _router_regex(char const* str, size_t n)
{
    return router_regex(std::string(str, n));
}

StartsWithRoutingTable::Item* StartsWithRoutingTable::Match(std::string const& url)
{
    auto it = _map.begin();
    
    for (; it != _map.end(); ++it)
    {
        if (Utilities::iStartsWith(url, it->first))
        {
            return &(it->second);
        }
    }
    
    return nullptr;
}

std::string to_parameter_pattern(std::string simple_pattern
                                 , std::vector<std::string> names)
{
    std::regex pattern("\\{\\{\\w*\\}\\}");
    
    std::string::const_iterator itb = simple_pattern.begin(), ite = simple_pattern.end();
    
    using PairItem = std::pair<std::string::const_iterator, std::string::const_iterator>;
    std::vector<PairItem> pos;
    std::smatch result;
    
    while (std::regex_search(itb, ite, result, pattern))
    {
        int const parameter_name_flag = 2;
        std::string name(result[0].first + parameter_name_flag, result[0].second - parameter_name_flag);
        names.push_back(name);
        
        PairItem it = std::make_pair(result[0].first, result[0].second);
        pos.push_back(it);
        
        itb = result[0].second;
    }
    
    for (size_t i = pos.size(); i > 0; --i)
    {
        auto item = pos[i - 1];
        simple_pattern.replace(item.first, item.second, "(\\w*)");
    }
    
    return simple_pattern;
}

bool RegexMatchRoutingTable::Insert(std::string const&  url_matcher, RouterItem const& item)
{
    Item ri(item);
    
    ri.url_matcher = url_matcher;
    
    try
    {
        ri.regex_matcher = to_parameter_pattern(url_matcher, ri.parameters);
        ri.regex_pattern.assign(ri.regex_matcher);
        _lst.push_back(ri);
        return true;
    }
    catch (std::regex_error const& e)
    {
        return false;
    }
    catch (std::exception const& e)
    {
        return false;
    }
}

RegexMatchRoutingTable::Item* RegexMatchRoutingTable::Exists(std::string const& url_matcher)
{
    for (auto& item : _lst)
    {
        if (url_matcher == item.url_matcher)
        {
            return &(item);
        }
    }
}

RegexMatchRoutingTable::Item* RegexMatchRoutingTable::Match(std::string const& url)
{
    try
    {
        for (auto& item : _lst)
        {
            if (std::regex_match(url, item.regex_pattern))
            {
                return &item;
            }
        }
    }
    catch (std::regex_error const& e)
    {
        return nullptr;
    }
    catch (std::exception const& e)
    {
        return nullptr;
    }
}

} //namespace da4qi4
