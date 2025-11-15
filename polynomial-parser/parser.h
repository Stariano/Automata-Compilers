#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"
#include <vector>
#include <map> // Include the map header
#include <set>

// Forward declarations
class Parser;
struct Value;
struct PolyEval;
struct Term;

// Represents the body of a polynomial as a list of terms.
// Each term is associated with an operator (+ or -).
struct PolyBody
{
    std::vector<std::pair<char, Term *>> terms;
};

// Represents a term in a polynomial expression.
// A term can be a coefficient with a list of monomials,
// or a more complex structure involving parenthesized expressions.
struct Term
{
    int coefficient;
    std::vector<int> monomial_power_vector;             
    std::vector<PolyBody *> parenthesized_lists;        // For products of parenthesized expressions
};

struct Polynomial
{
    std::string name;
    std::vector<std::string> parameters;
    int line_no;
    PolyBody *body; // Representation of the polynomial body for evaluation
    bool explicit_parameters;
};

enum ValueType
{
    NUM_TYPE,
    ID_TYPE,
    POLY_EVAL_TYPE
};

// Represents a value, which can be a number, a variable, or a polynomial evaluation.
struct Value
{
    ValueType type;
    int value;           // For NUM_TYPE
    std::string id;      // For ID_TYPE
    PolyEval *poly_eval; // For POLY_EVAL_TYPE
};

// Represents a polynomial evaluation, e.g., F(arg1, arg2)
struct PolyEval
{
    std::string poly_name;
    std::vector<Value *> arguments;
};

enum StatementType
{
    INPUT_STMT,
    OUTPUT_STMT,
    ASSIGN_STMT
};

// Represents a single statement in the EXECUTE section.
struct StatementNode
{
    StatementType type;
    std::string id;          // Variable name for INPUT, OUTPUT, and LHS of ASSIGN
    PolyEval *poly_eval; // For ASSIGN_STMT
};

// --- Execution and Evaluation Functions ---
int eval_poly(PolyEval *pe, Parser *parser);
int eval_poly_body(PolyBody *body, const std::vector<std::string> &params, const std::map<std::string, int> &arg_values, Parser *parser);
int eval_term(Term *term, const std::vector<std::string> &params, const std::map<std::string, int> &arg_values, Parser *parser);
int eval_value(Value *v, Parser *parser);

class Parser
{
    friend int eval_poly(PolyEval *pe, Parser *parser);
    friend int eval_value(Value *v, Parser *parser);

public:
    void ParseInput();

private:
    LexicalAnalyzer lexer;
    void parse_id_list(std::vector<std::string> &params);
    void syntax_error();
    void report_errors();
    Token expect(TokenType expected_type);
    std::map<std::string, Polynomial> symbolTable;
    std::map<std::string, std::vector<int>> errors;

    std::vector<std::string> poly_order;
    std::vector<int> tasks;
    std::vector<StatementNode *> program; // IR for EXECUTE section
    std::vector<int> inputs;              // Stored inputs from INPUTS section
    std::map<std::string, int> location_table;
    std::vector<int> mem;
    int next_location = 0;
    int next_input = 0;

    // --- Execution Helper ---
    void execute_program();
    int get_location(const std::string &id);

    // --- Printing Functions ---
    void print_poly_task3(const Polynomial &poly);
    void print_poly_body_task3(const PolyBody *body, const Polynomial &poly);
    void print_poly_task4(const Polynomial &poly);
    void print_poly_body_task4(PolyBody *body, const Polynomial &poly);
    void print_poly_task5(const Polynomial &poly);
    PolyBody* expand_poly_body(PolyBody *body, const Polynomial &poly);

    // Add declarations for your parsing functions here
    void parse_program();
    void parse_num_list();
    void parse_num_list(std::vector<int> &list);
    void parse_tasks_section();
    void parse_poly_section();
    void parse_poly_decl_list();
    void parse_poly_decl();
    void parse_poly_header(Polynomial &poly);
    PolyBody *parse_poly_body(const Polynomial &poly);
    void parse_term_list_prime(const Polynomial &poly, PolyBody *body);
    Term *parse_term(const Polynomial &poly);
    void parse_parenthesized_list(const Polynomial &poly, Term *term);
    void parse_monomial_list(const Polynomial &poly, Term *term);
    void parse_monomial(const Polynomial &poly, Term *term);
    void parse_exponent(int &exp);

    void parse_execute_section();
    void parse_statement_list();
    void parse_statement();
    void parse_input_statement();
    void parse_output_statement();
    void parse_assign_statement();
    PolyEval *parse_poly_evaluation();
    void parse_argument_list(std::vector<Value *> &args);
    Value *parse_argument();
    void parse_inputs_section(); // For the final section
};

#endif
