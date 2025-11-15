#include <iostream>
#include <algorithm>
#include <cstdlib>
#include "parser.h"
#include <set>
#include <cmath>

using namespace std;

void Parser::ParseInput()
{
    parse_program();
    expect(END_OF_FILE); // Ensures we consume the entire input
    if (!errors.empty())
    {
        report_errors();
    }
    else
    {
        bool execute = false;
        for (int task : tasks)
        {
            if (task == 2)
            {
                execute = true;
                break;
            }
        }
        if (execute)
        {
            execute_program();
        }

        bool task3 = false;
        for (int task : tasks)
        {
            if (task == 3)
            {
                task3 = true;
                break;
            }
        }
        if (task3)
        {
            cout << "POLY - SORTED MONOMIAL LISTS" << endl;
            for (const auto &poly_name : poly_order)
            {
                print_poly_task3(symbolTable.at(poly_name));
            }
        }

        bool task4 = false;
        for (int task : tasks)
        {
            if (task == 4)
            {
                task4 = true;
                break;
            }
        }
        if (task4)
        {
            cout << "POLY - COMBINED MONOMIAL LISTS" << endl;
            for (const auto &poly_name : poly_order)
            {
                print_poly_task4(symbolTable.at(poly_name));
            }
        }

        bool task5 = false;
        for (int task : tasks)
        {
            if (task == 5)
            {
                task5 = true;
                break;
            }
        }
        if (task5)
        {
            cout << "POLY - EXPANDED" << endl;
            for (const auto &poly_name : poly_order)
            {
                print_poly_task5(symbolTable.at(poly_name));
            }
        }
    }
}

// ... main function ...

int main()
{
    Parser parser;
    parser.ParseInput();

    return 0;
}