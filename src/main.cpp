#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <type_traits>
#include <format>
#include <map>

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

const string html_big =
    R"(
    <html>
    <div>
        <input/>
        <p>Text</p>
        <p>Text</p>
        <p>Text</p>
        <p>Text</p>
        <div>
            <p>Text</p>
            <p>Text</p>
            <p>Text</p>
            <p>Text</p>
        </div>
    </div>
    <div></div>
    </html>
    )";

const string html_small =
    R"(
    <html>
    </html>
    )";

string html = html_big;

struct DOM
{
    DOM *parent;
    vector<DOM *> children;
    string tag;
    string id;
    string className;
    string text;
    map<string, string> kv;
    bool closed = false; // <input/> doesn't has inner content
};

struct Brackets
{
    int idx;
    bool close = false;
};

template <typename T>
auto to_native(const T &val)
{
    if constexpr (std::is_same_v<string, T>)
        return val.c_str();
    else
        return val;
}

template <typename... Args>
void print(const char *ln, Args... args)
{
    printf(ln, to_native(args)...);
}

void PrintDOM(DOM *dom, string tabs = "")
{
    if (dom->closed)
    {
        std::cout << std::format("{} <{}/>\n", tabs, dom->tag);
        return;
    }

    if (dom->children.empty())
    {
        std::cout << std::format("{} <{}></{}>\n", tabs, dom->tag, dom->tag);
        return;
    }

    std::cout << std::format("{} <{}>\n", tabs, dom->tag);

    for (const auto &child : dom->children)
    {
        PrintDOM(child, tabs + "   ");
    }

    std::cout << std::format("{} </{}>\n", tabs, dom->tag);
}

struct File
{
    uint8_t *_data = nullptr;
    int _size = 0;

    bool readAll(const char *filename)
    {
        FILE *f;
        if (fopen_s(&f, filename, "rb") != 0)
            return false;
        fseek(f, 0, SEEK_END);
        _size = ftell(f);
        fseek(f, 0, SEEK_SET);
        _data = new uint8_t[_size];
        fread(_data, _size, 1, f);
        fclose(f);
        return true;
    }

    ~File()
    {
        if (_data)
            delete[] _data;
    }
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
    //   << "id:'" << dom->id << "'\n"
    //   << "class:'" << dom->className << "'\n";
    for (const auto &[key, value] : dom->kv)
        std::cout << "'" << key << "':'" << value << "'\n";
    printf("\n");
}

int main()
{
    {DOM dom; parse_tag(&dom, R"(div className="wrapper" id="global_wrapper")"); print_tag(&dom);}
    {DOM dom; parse_tag(&dom, R"()"); print_tag(&dom);}
    {DOM dom; parse_tag(&dom, R"(a)"); print_tag(&dom);}
    {DOM dom; parse_tag(&dom, R"(input id="list" value="" name="Ivan")"); print_tag(&dom);}
    {DOM dom; parse_tag(&dom, R"(youtube-player-view)"); print_tag(&dom);}

    return 0;

    const char *htmlFilename = "D:/Common/Code/Web/Next/profi_steps_v3/out/index.html";
    File htmlFile;
    if (!htmlFile.readAll(htmlFilename))
    {
        std::cout << "File open failed!" << "\n";
        return 1;
    }
    // std::cout << (char*)htmlFile._data << "\n"; return 0;
    html = string{(char *)htmlFile._data};

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
                    tmp->closed = true;
                    tmp->tag = tag;
                    tmp->parent = curr;
                    curr->children.push_back(tmp);
                }
                else
                {
                    string tag = string_utils::trim(html.substr(b.idx + 1, i - b.idx - 1));
                    DOM *tmp = new DOM;
                    tmp->tag = tag;
                    tmp->parent = curr;
                    curr->children.push_back(tmp);
                    curr = tmp;
                }
            }
        }
    }
    PrintDOM(curr);
    return 0;
}
