#include "parser.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cmath>

using namespace std;

void Parser::print_poly_body_task3(const PolyBody *body, const Polynomial &poly)
{
    for (size_t i = 0; i < body->terms.size(); ++i)
    {
        const auto &pair = body->terms[i];
        char op = pair.first;
        Term *term = pair.second;

        if (i > 0)
        {
            cout << " " << op << " ";
        }
        else if (op == '-')
        {
            cout << "-";
        }

        if (!term->parenthesized_lists.empty())
        {
            for (const auto &sub_body : term->parenthesized_lists)
            {
                cout << "(";
                print_poly_body_task3(sub_body, poly);
                cout << ")";
            }
        }
        else
        {
            bool is_constant = true;
            for (int power : term->monomial_power_vector)
            {
                if (power > 0)
                {
                    is_constant = false;
                    break;
                }
            }

            if (abs(term->coefficient) != 1 || is_constant)
            {
                cout << abs(term->coefficient);
            }

            for (size_t j = 0; j < term->monomial_power_vector.size(); ++j)
            {
                if (term->monomial_power_vector[j] > 0)
                {
                    cout << poly.parameters[j];
                    if (term->monomial_power_vector[j] > 1)
                    {
                        cout << "^" << term->monomial_power_vector[j];
                    }
                }
            }
        }
    }
}

void Parser::print_poly_task3(const Polynomial &poly)
{
    cout << "    " << poly.name;
    if (poly.explicit_parameters)
    {
        cout << "(";
        for (size_t i = 0; i < poly.parameters.size(); ++i)
        {
            cout << poly.parameters[i] << (i == poly.parameters.size() - 1 ? "" : ",");
        }
        cout << ")";
    }
    cout << " = ";
    print_poly_body_task3(poly.body, poly);
    cout << " ;" << endl;
}

void Parser::print_poly_task4(const Polynomial &poly)
{
    cout << "    " << poly.name;
    if (poly.explicit_parameters)
    {
        cout << "(";
        for (size_t i = 0; i < poly.parameters.size(); ++i)
        {
            cout << poly.parameters[i] << (i == poly.parameters.size() - 1 ? "" : ",");
        }
        cout << ")";
    }
    cout << " = ";
    print_poly_body_task4(poly.body, poly);
    cout << " ;" << endl;
}

void Parser::print_poly_body_task4(PolyBody *body, const Polynomial &poly)
{
    map<vector<int>, int> combined_coeffs;
    vector<Term *> print_order;
    map<Term *, bool> is_paren_list;

    // First pass: combine monomial lists and establish print order
    for (const auto &pair : body->terms)
    {
        Term *term = pair.second;
        if (!term->parenthesized_lists.empty())
        {
            print_order.push_back(term);
            is_paren_list[term] = true;
        }
        else
        {
            int sign = (pair.first == '+') ? 1 : -1;
            if (combined_coeffs.find(term->monomial_power_vector) == combined_coeffs.end())
            {
                // First time we see this monomial list, add to print order
                print_order.push_back(term);
                is_paren_list[term] = false;
                combined_coeffs[term->monomial_power_vector] = 0; // Initialize
            }
            combined_coeffs[term->monomial_power_vector] += sign * term->coefficient;
        }
    }

    bool first_term_printed = false;
    for (Term *term_to_print : print_order)
    {
        if (is_paren_list[term_to_print])
        {
            // This is a parenthesized list, print it recursively.
            char op = '+'; // Find original operator
            for(const auto& pair : body->terms) {
                if (pair.second == term_to_print) {
                    op = pair.first;
                    break;
                }
            }

            if (first_term_printed)
            {
                cout << " " << op << " ";
            }
            else if (op == '-')
            {
                cout << "-";
            }

            for (size_t i = 0; i < term_to_print->parenthesized_lists.size(); ++i)
            {
                cout << "(";
                print_poly_body_task4(term_to_print->parenthesized_lists[i], poly);
                cout << ")";
            }
            first_term_printed = true;
        }
        else
        {
            // This is a monomial list, print its combined form.
            int total_coefficient = combined_coeffs[term_to_print->monomial_power_vector];
            if (total_coefficient == 0) continue;

            char op = (total_coefficient > 0) ? '+' : '-';
            int abs_coefficient = abs(total_coefficient);

            if (first_term_printed)
            {
                cout << " " << op << " ";
            }
            else if (op == '-')
            {
                cout << "-";
            }

            const auto &power_vector = term_to_print->monomial_power_vector;
            bool is_constant = true;
            for (int power : power_vector)
            {
                if (power > 0)
                {
                    is_constant = false;
                    break;
                }
            }

            if (abs_coefficient != 1 || is_constant)
            {
                cout << abs_coefficient;
            }

            for (size_t j = 0; j < power_vector.size(); ++j)
            {
                if (power_vector[j] > 0)
                {
                    cout << poly.parameters[j];
                    if (power_vector[j] > 1)
                    {
                        cout << "^" << power_vector[j];
                    }
                }
            }
            first_term_printed = true;
        }
    }
}

void Parser::print_poly_task5(const Polynomial &poly)
{
    cout << "    " << poly.name;
    if (poly.explicit_parameters)
    {
        cout << "(";
        for (size_t i = 0; i < poly.parameters.size(); ++i)
        {
            cout << poly.parameters[i] << (i == poly.parameters.size() - 1 ? "" : ",");
        }
        cout << ")";
    }
    cout << " = ";
    PolyBody *expanded_body = expand_poly_body(poly.body, poly);
    
    map<vector<int>, int> combined_terms;
    for (const auto &pair : expanded_body->terms)
    {
        int sign = (pair.first == '+') ? 1 : -1;
        combined_terms[pair.second->monomial_power_vector] += sign * pair.second->coefficient;
    }

    vector<pair<vector<int>, int>> sorted_terms;
    for (auto const &term : combined_terms)
    {
        if (term.second != 0)
        {
            sorted_terms.push_back(term);
        }
    }

    sort(sorted_terms.begin(), sorted_terms.end(), [](const pair<vector<int>, int> &a, const pair<vector<int>, int> &b) {
        int degree_a = 0;
        for (int p : a.first) degree_a += p;
        int degree_b = 0;
        for (int p : b.first) degree_b += p;

        if (degree_a != degree_b)
        {
            return degree_a > degree_b;
        }
        return a.first > b.first;
    });

    bool first_term = true;
    for (const auto &term : sorted_terms)
    {
        const auto &power_vector = term.first;
        int total_coefficient = term.second;
        char op = (total_coefficient > 0) ? '+' : '-';
        int abs_coefficient = abs(total_coefficient);

        if (!first_term)
        {
            cout << " " << op << " ";
        }
        else if (op == '-')
        {
            cout << "-";
        }

        bool is_constant = true;
        for (int power : power_vector)
        {
            if (power > 0)
            {
                is_constant = false;
                break;
            }
        }

        if (abs_coefficient != 1 || is_constant)
        {
            cout << abs_coefficient;
        }

        for (size_t j = 0; j < power_vector.size(); ++j)
        {
            if (power_vector[j] > 0)
            {
                cout << poly.parameters[j];
                if (power_vector[j] > 1)
                {
                    cout << "^" << power_vector[j];
                }
            }
        }
        first_term = false;
    }

    cout << " ;" << endl;
}

PolyBody* Parser::expand_poly_body(PolyBody *body, const Polynomial &poly)
{
    PolyBody *expanded_body = new PolyBody();

    for (auto &term_pair : body->terms)
    {
        char op = term_pair.first;
        Term *term = term_pair.second;

        if (term->parenthesized_lists.empty())
        {
            expanded_body->terms.push_back({op, term});
        }
        else
        {
            PolyBody *current_expansion = new PolyBody();
            Term *initial_term = new Term();
            initial_term->coefficient = 1;
            initial_term->monomial_power_vector.resize(poly.parameters.size(), 0);
            current_expansion->terms.push_back({'+', initial_term});

            for (auto &sub_body : term->parenthesized_lists)
            {
                PolyBody *expanded_sub_body = expand_poly_body(sub_body, poly);
                PolyBody *next_expansion = new PolyBody();
                
                for (auto const &t1 : current_expansion->terms)
                {
                    for (auto const &t2 : expanded_sub_body->terms)
                    {
                        Term *new_term = new Term();
                        new_term->coefficient = t1.second->coefficient * t2.second->coefficient;
                        
                        new_term->monomial_power_vector.resize(poly.parameters.size());
                        for (size_t i = 0; i < poly.parameters.size(); ++i)
                        {
                            new_term->monomial_power_vector[i] = t1.second->monomial_power_vector[i] + t2.second->monomial_power_vector[i];
                        }
                        
                        char new_op = (t1.first == t2.first) ? '+' : '-';
                        next_expansion->terms.push_back({new_op, new_term});
                    }
                }
                current_expansion = next_expansion;
            }
            
            for (auto &new_term_pair : current_expansion->terms)
            {
                new_term_pair.second->coefficient *= term->coefficient;
                char final_op = (op == new_term_pair.first) ? '+' : '-';
                expanded_body->terms.push_back({final_op, new_term_pair.second});
            }
        }
    }

    return expanded_body;
}
