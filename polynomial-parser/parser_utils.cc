#include <iostream>
#include <algorithm>
#include <cstdlib>
#include "parser.h"
#include <vector>
#include <map>

using namespace std;

void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!!!&%!!!!&%!!!!!!" << endl;
    exit(1);
}

Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

void Parser::report_errors()
{
    if (errors.empty())
    {
        return; // No errors to report
    }

    // The map iterator will give us errors in alphabetical order by key
    for (map<string, vector<int>>::const_iterator it = errors.begin(); it != errors.end(); ++it)
    {
        cout << "Semantic Error Code " << it->first << ": ";
        vector<int> line_nums = it->second;
        sort(line_nums.begin(), line_nums.end());
        for (size_t i = 0; i < line_nums.size(); ++i)
        {
            cout << line_nums[i] << (i == line_nums.size() - 1 ? "" : " ");
        }
        cout << endl;
    }
    exit(1); // Exit after reporting errors
}

int Parser::get_location(const std::string &id)
{
    if (location_table.count(id) == 0)
    {
        location_table[id] = next_location;
        mem.push_back(0); // Initialize memory to 0
        next_location++;
    }
    return location_table[id];
}
