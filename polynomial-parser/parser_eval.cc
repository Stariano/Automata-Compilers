#include "parser.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <map>

using namespace std;

void Parser::execute_program()
{
    for (StatementNode *stmt : program)
    {
        int location = get_location(stmt->id);

        switch (stmt->type)
        {
        case INPUT_STMT:
            if (next_input < inputs.size())
            {
                mem[location] = inputs[next_input];
                next_input++;
            }
            else
            {
                // Handle case where there are not enough inputs.
                // The spec implies we can assume enough inputs are provided.
            }
            break;
        case OUTPUT_STMT:
            cout << mem[location] << endl;
            break;
        case ASSIGN_STMT:
            mem[location] = eval_poly(stmt->poly_eval, this);
            break;
        }
    }
}

int eval_value(Value *v, Parser *parser)
{
    switch (v->type)
    {
    case NUM_TYPE:
        return v->value;
    case ID_TYPE:
        return parser->mem[parser->get_location(v->id)];
    case POLY_EVAL_TYPE:
        return eval_poly(v->poly_eval, parser);
    }
    return 0; // Should not happen
}

// Forward declare eval_poly_body for eval_term
int eval_poly_body(PolyBody *body, const std::vector<std::string> &params, const std::map<std::string, int> &arg_values, Parser *parser);

int eval_term(Term *term, const std::vector<std::string> &params, const std::map<std::string, int> &arg_values, Parser *parser)
{
    int term_val = 1;
    // Handle parenthesized lists product
    for (PolyBody *pb : term->parenthesized_lists)
    {
        term_val *= eval_poly_body(pb, params, arg_values, parser);
    }

    int monomial_product = 1;
    for (size_t i = 0; i < term->monomial_power_vector.size(); ++i)
    {
        if (term->monomial_power_vector[i] > 0)
        {
            std::string var_name = params[i];
            int power = term->monomial_power_vector[i];
            int base = arg_values.count(var_name) ? arg_values.at(var_name) : 0;
            
            // Integer exponentiation to avoid floating-point errors
            long long term_pow = 1;
            for(int j = 0; j < power; ++j) {
                term_pow *= base;
            }
            monomial_product *= term_pow;
        }
    }
    term_val *= term->coefficient * monomial_product;

    return term_val;
}

int eval_poly_body(PolyBody *body, const std::vector<std::string> &params, const std::map<std::string, int> &arg_values, Parser *parser)
{
    int total = 0;
    for (const auto &pair : body->terms)
    {
        char op = pair.first;
        Term *term = pair.second;
        int term_value = eval_term(term, params, arg_values, parser);

        if (op == '+')
        {
            total += term_value;
        }
        else // op == '-'
        {
            total -= term_value;
        }
    }
    return total;
}

int eval_poly(PolyEval *pe, Parser *parser)
{
    // 1. Find the polynomial declaration
    if (parser->symbolTable.count(pe->poly_name) == 0)
    {
        return 0; // Safeguard
    }
    Polynomial poly = parser->symbolTable.at(pe->poly_name);

    // 2. Evaluate arguments and create a mapping from parameter name to value
    std::map<std::string, int> arg_values;
    for (size_t i = 0; i < pe->arguments.size(); ++i)
    {
        std::string param_name = poly.parameters[i];
        arg_values[param_name] = eval_value(pe->arguments[i], parser);
    }

    // 3. Evaluate the polynomial body
    return eval_poly_body(poly.body, poly.parameters, arg_values, parser);
}
