#include "parser.h"
#include <vector>
#include <string>

void Parser::parse_program()
{
    // program -> tasks_section poly_section execute_section inputs_section

    parse_tasks_section();
    parse_poly_section();
    parse_execute_section();
    parse_inputs_section();
}

void Parser::parse_tasks_section()
{
    // tasks_section -> TASKS num_list
    expect(TASKS);
    parse_num_list(tasks);
}

void Parser::parse_num_list(std::vector<int> &list)
{
    // This means we must see at least one NUM
    Token num_token = expect(NUM);
    list.push_back(std::stoi(num_token.lexeme));

    // After the first NUM, we check if there's another one.
    Token t = lexer.peek(1);
    if (t.token_type == NUM)
    {
        parse_num_list(list); // Recursive call for the rest of the list
    }
}
void Parser::parse_num_list()
{
    // Grammar for num_list:
    // num_list -> NUM
    // num_list -> NUM num_list

    // This means we must see at least one NUM
    expect(NUM);

    // After the first NUM, we check if there's another one.
    // The peek() function is perfect for this. It lets us look ahead
    // at the next token without consuming it.
    Token t = lexer.peek(1);
    if (t.token_type == NUM)
    {
        parse_num_list(); // Recursive call for the rest of the list
    }
}

void Parser::parse_inputs_section()
{
    // inputs_section -> INPUTS num_list
    expect(INPUTS);
    parse_num_list(inputs);
}

void Parser::parse_id_list(std::vector<std::string> &params)
{
    // id_list -> ID
    // id_list -> ID COMMA id_list
    // This grammar does not allow an empty id_list.

    // Expect at least one ID.
    Token id_token = expect(ID);
    params.push_back(id_token.lexeme);

    // After the first ID, we might see a comma for the next one.
    Token t = lexer.peek(1);
    if (t.token_type == COMMA)
    {
        expect(COMMA);
        parse_id_list(params); // Recursively parse the rest of the list.
    }
}
