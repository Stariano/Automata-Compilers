#include "parser.h"

void Parser::parse_poly_section()
{
    // poly_section -> POLY poly_decl_list
    expect(POLY);
    parse_poly_decl_list();
}

void Parser::parse_poly_decl_list()
{
    // poly_decl_list -> poly_decl | poly_decl poly_decl_list
    parse_poly_decl();

    // Check if another declaration follows
    Token t = lexer.peek(1);
    if (t.token_type == ID)
    {
        parse_poly_decl_list();
    }
}
void Parser::parse_poly_decl()
{
    Polynomial new_poly;
    new_poly.explicit_parameters = false;
    parse_poly_header(new_poly); // Parse the header first

    // --- SEMANTIC CHECK for Redeclaration ---
    if (symbolTable.count(new_poly.name) > 0)
    {
        errors["DMT-12"].push_back(new_poly.line_no);
        // We can skip parsing the body of a re-declared polynomial
        // by consuming tokens until the semicolon.
        Token t = lexer.peek(1);
        while (t.token_type != SEMICOLON && t.token_type != END_OF_FILE)
        {
            lexer.GetToken();
            t = lexer.peek(1);
        }
    }
    else
    {
        // Add to symbol table BEFORE parsing body.
        symbolTable[new_poly.name] = new_poly;
        poly_order.push_back(new_poly.name); // Store declaration order
        expect(EQUAL);
        // Now parse the body and update the entry in the symbol table.
        new_poly.body = parse_poly_body(new_poly);
        symbolTable[new_poly.name] = new_poly;
    }
    expect(SEMICOLON);
}

void Parser::parse_poly_header(Polynomial &poly)
{
    Token poly_name = expect(ID);
    poly.name = poly_name.lexeme;
    poly.line_no = poly_name.line_no;

    Token t = lexer.peek(1);
    if (t.token_type == LPAREN)
    {
        poly.explicit_parameters = true;
        expect(LPAREN);
        parse_id_list(poly.parameters);
        expect(RPAREN);
    }
    else
    {
        // Default parameter for univariate polynomials
        poly.parameters.push_back("x");
    }
}

PolyBody *Parser::parse_poly_body(const Polynomial &poly)
{
    PolyBody *body = new PolyBody();
    body->terms.push_back({'+', parse_term(poly)}); // First term is always positive
    parse_term_list_prime(poly, body);
    return body;
}

void Parser::parse_term_list_prime(const Polynomial &poly, PolyBody *body)
{
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS)
    {
        char op = (t.token_type == PLUS) ? '+' : '-';
        expect(t.token_type); // Consume PLUS or MINUS
        body->terms.push_back({op, parse_term(poly)});
        parse_term_list_prime(poly, body);
    }
}

Term *Parser::parse_term(const Polynomial &poly)
{
    Term *term = new Term();
    term->monomial_power_vector.resize(poly.parameters.size(), 0);

    Token t = lexer.peek(1);
    if (t.token_type == NUM)
    {
        // term -> coefficient
        // term -> coefficient monomial_list
        Token num_token = expect(NUM);
        term->coefficient = std::stoi(num_token.lexeme);

        Token t2 = lexer.peek(1);
        if (t2.token_type == ID)
        {
            parse_monomial_list(poly, term);
        }
    }
    else if (t.token_type == ID)
    {
        // term -> monomial_list
        term->coefficient = 1; // Default coefficient
        parse_monomial_list(poly, term);
    }
    else if (t.token_type == LPAREN)
    {
        // term -> parenthesized_list
        term->coefficient = 1; // Default coefficient for parenthesized lists
        parse_parenthesized_list(poly, term);
    }
    else
    {
        syntax_error();
    }
    return term;
}

void Parser::parse_parenthesized_list(const Polynomial &poly, Term *term)
{
    // parenthesized_list -> LPAREN term_list RPAREN
    // parenthesized_list -> LPAREN term_list RPAREN parenthesized_list

    expect(LPAREN);
    term->parenthesized_lists.push_back(parse_poly_body(poly));
    expect(RPAREN);

    Token t = lexer.peek(1);
    if (t.token_type == LPAREN)
    {
        parse_parenthesized_list(poly, term);
    }
}

void Parser::parse_monomial_list(const Polynomial &poly, Term *term)
{
    parse_monomial(poly, term);

    Token t = lexer.peek(1);
    if (t.token_type == ID)
    {
        parse_monomial_list(poly, term);
    }
}

void Parser::parse_monomial(const Polynomial &poly, Term *term)
{
    Token id_token = expect(ID);
    int exp = 1;
    parse_exponent(exp);

    // --- IM-4 CHECK ---
    bool found = false;
    int param_index = -1;
    for (size_t i = 0; i < poly.parameters.size(); ++i)
    {
        if (poly.parameters[i] == id_token.lexeme)
        {
            found = true;
            param_index = i;
            break;
        }
    }

    if (!found)
    {
        errors["IM-4"].push_back(id_token.line_no);
    }
    else
    {
        // Add exponent to the corresponding index in the power vector
        term->monomial_power_vector[param_index] += exp;
    }
    // --- END CHECK ---
}

void Parser::parse_exponent(int &exp)
{
    Token t = lexer.peek(1);
    if (t.token_type == POWER)
    {
        expect(POWER);
        Token num = expect(NUM);
        exp = std::stoi(num.lexeme);
    }
    else
    {
        exp = 1; // Default exponent
    }
}
