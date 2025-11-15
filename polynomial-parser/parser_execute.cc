#include "parser.h"
#include <vector>

void Parser::parse_execute_section()
{
    // execute_section -> EXECUTE statement_list
    expect(EXECUTE);
    parse_statement_list();
}

void Parser::parse_statement_list()
{
    // statement_list -> statement | statement statement_list
    parse_statement();

    // A statement can start with INPUT, OUTPUT, or an ID.
    // We peek ahead to see if another statement follows.
    Token t = lexer.peek(1);
    if (t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID)
    {
        parse_statement_list();
    }
}

void Parser::parse_statement()
{
    // statement -> input_statement | output_statement | assign_statement
    Token t = lexer.peek(1);
    if (t.token_type == INPUT)
    {
        parse_input_statement();
    }
    else if (t.token_type == OUTPUT)
    {
        parse_output_statement();
    }
    else if (t.token_type == ID)
    {
        parse_assign_statement();
    }
    else
    {
        syntax_error();
    }
}

void Parser::parse_input_statement()
{
    expect(INPUT);
    Token id_token = expect(ID);
    expect(SEMICOLON);

    StatementNode *stmt = new StatementNode();
    stmt->type = INPUT_STMT;
    stmt->id = id_token.lexeme;
    get_location(id_token.lexeme); // Ensure variable is allocated
    program.push_back(stmt);
}

void Parser::parse_output_statement()
{
    // output_statement -> OUTPUT ID SEMICOLON
    expect(OUTPUT);
    Token id_token = expect(ID);
    expect(SEMICOLON);

    StatementNode *stmt = new StatementNode();
    stmt->type = OUTPUT_STMT;
    stmt->id = id_token.lexeme;
    get_location(id_token.lexeme); // Ensure variable is allocated
    program.push_back(stmt);
}

void Parser::parse_assign_statement()
{
    Token id_token = expect(ID);
    expect(EQUAL);
    PolyEval *poly_eval = parse_poly_evaluation();
    expect(SEMICOLON);

    StatementNode *stmt = new StatementNode();
    stmt->type = ASSIGN_STMT;
    stmt->id = id_token.lexeme;
    stmt->poly_eval = poly_eval;
    get_location(id_token.lexeme); // Ensure variable is allocated
    program.push_back(stmt);
}

PolyEval *Parser::parse_poly_evaluation()
{
    Token poly_name = expect(ID);
    expect(LPAREN);
    std::vector<Value *> args;
    parse_argument_list(args);
    expect(RPAREN);

    // --- SEMANTIC CHECK for AUP-13 & NA-7 ---
    if (symbolTable.count(poly_name.lexeme) == 0)
    {
        // Undeclared polynomial
        errors["AUP-13"].push_back(poly_name.line_no);
    }
    else
    {
        // Declared, so check arg count
        if (symbolTable.at(poly_name.lexeme).parameters.size() != args.size())
        {
            errors["NA-7"].push_back(poly_name.line_no);
        }
    }
    // --- END SEMANTIC CHECK ---

    PolyEval *pe = new PolyEval();
    pe->poly_name = poly_name.lexeme;
    pe->arguments = args;
    return pe;
}
void Parser::parse_argument_list(std::vector<Value *> &args)
{
    args.push_back(parse_argument());

    Token t = lexer.peek(1);
    if (t.token_type == COMMA)
    {
        expect(COMMA);
        parse_argument_list(args); // Recurse
    }
}

Value *Parser::parse_argument()
{
    // argument -> ID | NUM | poly_evaluation
    Token t = lexer.peek(1);
    Value *val = new Value();

    if (t.token_type == ID)
    {
        Token t2 = lexer.peek(2);
        if (t2.token_type == LPAREN)
        {
            val->type = POLY_EVAL_TYPE;
            val->poly_eval = parse_poly_evaluation();
        }
        else
        {
            Token id_token = expect(ID); // Consume the ID
            val->type = ID_TYPE;
            val->id = id_token.lexeme;
            get_location(id_token.lexeme); // Ensure variable is allocated
        }
    }
    else if (t.token_type == NUM)
    {
        Token num_token = expect(NUM);
        val->type = NUM_TYPE;
        val->value = std::stoi(num_token.lexeme);
    }
    else
    {
        syntax_error();
    }
    return val;
}
