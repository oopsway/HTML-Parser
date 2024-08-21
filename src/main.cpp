#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <type_traits>
#include <format>
#include <map>
#include "file.hpp"

using std::format;
using std::map;
using std::stack;
using std::string;
using std::vector;

namespace string_utils
{
    string trim(const string &s)
    {
        int l = 0, r = s.size();
        while (l < r && isspace(s[l]))
            l++;
        if (l >= r)
            return {};
        while (r >= 1 && isspace(s[r - 1]))
            r--;
        if (r <= 0)
            return {};
        return s.substr(l, r - l);
    }
}

string html = "<div></div>";

struct DOM
{
    DOM *parent;
    vector<DOM *> children;
    string tag;
    string id;
    string className;
    string text;
    map<string, string> kv;
    bool closed = false;
};

struct Brackets
{
    int idx;
    bool close = false;
};

enum HtmlParserError
{
    NoError = 0,
    EmptyTag,
    IncompleteKeyName,
    IncompleteKeyValue,
    IncompleteValue,
};

// input like 'div className="wrapper" id="global_wrapper"'
HtmlParserError parse_tag(DOM *dom, const string &tag)
{
    const int n = tag.size();
    if (n == 0)
        return EmptyTag;

    int i = 0;
    while (i < n && tag[i] != ' ')
        i++;

    dom->tag = string_utils::trim(tag.substr(0, i));
    i++;
    if (i >= n)
        return EmptyTag;

    while (i < n)
    {
        int begin = i;
        while (i < n && tag[i] != '=')
            i++;
        if (i >= n)
            return IncompleteKeyName;

        const string key = string_utils::trim(tag.substr(begin, i - begin));

        while (i < n && tag[i] != '"')
            i++;
        if (i >= n)
            return IncompleteKeyValue;

        begin = ++i;

        while (i < n && tag[i] != '"')
            i++;
        if (i >= n)
            return IncompleteValue;
        const string value = string_utils::trim(tag.substr(begin, i - begin));
        dom->kv[key] = value;
        i++;
    }

    return NoError;
}

void print_tag(DOM *dom)
{
    std::cout << "'tag':'" << dom->tag << "'\n";
    for (const auto &[key, value] : dom->kv)
        std::cout << "'" << key << "':'" << value << "'\n";
    std::cout << "\n";
}

void print_dom(DOM *dom, string tabs = "")
{
    print_tag(dom);
    if (dom->closed)
    {
        // std::cout << std::format("{} <{}/>\n", tabs, dom->tag);
        return;
    }

    if (dom->children.empty())
    {
        // std::cout << std::format("{} <{}></{}>\n", tabs, dom->tag, dom->tag);
        return;
    }

    // std::cout << std::format("{} <{}>\n", tabs, dom->tag);
    for (const auto &child : dom->children)
    {
        print_dom(child, tabs + "   ");
    }

    // std::cout << std::format("{} </{}>\n", tabs, dom->tag);
}

int main()
{
    // {DOM dom; parse_tag(&dom, R"(div className="wrapper" id="global_wrapper")"); print_tag(&dom);}
    // {DOM dom; parse_tag(&dom, R"()"); print_tag(&dom);}
    // {DOM dom; parse_tag(&dom, R"(a)"); print_tag(&dom);}
    // {DOM dom; parse_tag(&dom, R"(input id="list" value="" name="Ivan")"); print_tag(&dom);}
    // {DOM dom; parse_tag(&dom, R"(custom-tag custom_value="234902349")"); print_tag(&dom);}
    // return 0;

    const char *htmlFilename = "C:/index.html";

    File htmlFile;
    if (!htmlFile.readAll(htmlFilename))
    {
        std::cout << "File open failed! - " << htmlFilename << "\n";
        return 1;
    }
    // std::cout << htmlFile.c_str() << "\n"; return 0;
    html = string{htmlFile.c_str()};
    htmlFile.clear();

    stack<Brackets> open_bracket;
    DOM root{};
    DOM *curr = &root;

    for (int i = 0; i < html.size(); i++)
    {
        if (html[i] == '<' && open_bracket.empty())
        {
            open_bracket.push({i, (html[i + 1] == '/')});
        }
        else if (html[i] == '>' && !open_bracket.empty())
        {
            auto b = open_bracket.top();
            open_bracket.pop();

            if (b.close)
            {
                curr = curr->parent;
            }
            else
            {
                if (html[i - 1] == '/')
                {
                    string tag = string_utils::trim(html.substr(b.idx + 1, i - b.idx - 2));
                    DOM *tmp = new DOM;
                    parse_tag(tmp, tag);
                    tmp->closed = true;
                    tmp->parent = curr;
                    curr->children.push_back(tmp);
                }
                else
                {
                    string tag = string_utils::trim(html.substr(b.idx + 1, i - b.idx - 1));
                    DOM *tmp = new DOM;
                    parse_tag(tmp, tag);
                    tmp->parent = curr;
                    curr->children.push_back(tmp);
                    curr = tmp;
                }
            }
        }
    }
    print_dom(curr);
    return 0;
}
