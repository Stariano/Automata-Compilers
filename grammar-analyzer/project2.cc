/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <deque>
#include <utility>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "lexer.h"

using namespace std;

struct Rule
{
    string lhs;
    vector<string> rhs;
};

static LexicalAnalyzer lexer;
static Token current_token;

static vector<Rule> grammar_rules;
static vector<string> symbol_order;
static vector<string> nonterminal_order;
static unordered_set<string> seen_symbols;
static unordered_set<string> nonterminal_symbols;
static string start_symbol;

[[noreturn]] static void SyntaxError()
{
    // bail out immediately so we never try to recover from malformed input
    cout << "SYNTAX ERROR !!!!!!!!!!!!!!";
    exit(1);
}

static void Advance()
{
    current_token = lexer.GetToken();
}

static void RecordSymbol(const string &name)
{
    if (seen_symbols.insert(name).second)
    {
        symbol_order.push_back(name);
    }
}

static void MarkNonterminal(const string &name)
{
    nonterminal_symbols.insert(name);
}

static bool IsNonterminal(const string &name)
{
    return nonterminal_symbols.find(name) != nonterminal_symbols.end();
}

static void Expect(TokenType expected)
{
    if (current_token.token_type != expected)
    {
        SyntaxError();
    }
    Advance();
}

static vector<string> ParseIdList()
{
    vector<string> rhs;
    while (current_token.token_type == ID)
    {
        RecordSymbol(current_token.lexeme);
        rhs.push_back(current_token.lexeme);
        Advance();
    }
    return rhs;
}

static void ParseRightHandSide(const string &lhs)
{
    // each OR-delimited alternative turns into its own rule entry
    grammar_rules.push_back({lhs, ParseIdList()});

    while (current_token.token_type == OR)
    {
        Advance();
        grammar_rules.push_back({lhs, ParseIdList()});
    }
}

static void ParseRuleList();

static void ParseRule()
{
    if (current_token.token_type != ID)
    {
        SyntaxError();
    }

    string lhs = current_token.lexeme;
    if (start_symbol.empty())
    {
        // first rule we see defines the start symbol (per project spec)
        start_symbol = lhs;
    }
    RecordSymbol(lhs);
    MarkNonterminal(lhs);
    Advance();

    Expect(ARROW);
    ParseRightHandSide(lhs);
    Expect(STAR);
}

static void ParseRuleList()
{
    ParseRule();

    while (current_token.token_type == ID)
    {
        ParseRule();
    }
}

static void ParseGrammar()
{
    ParseRuleList();

    if (current_token.token_type != HASH)
    {
        SyntaxError();
    }
    Advance();

    if (current_token.token_type != END_OF_FILE)
    {
        SyntaxError();
    }
}

// read grammar
void ReadGrammar()
{
    grammar_rules.clear();
    symbol_order.clear();
    nonterminal_order.clear();
    seen_symbols.clear();
    nonterminal_symbols.clear();
    start_symbol.clear();

    Advance();
    ParseGrammar();

    nonterminal_order.clear();
    for (const auto &name : symbol_order)
    {
        if (IsNonterminal(name))
        {
            nonterminal_order.push_back(name);
        }
    }
    // nonterminal_order now mirrors first-appearance order, which keeps output deterministic
}

/*
 * Task 1:
 * Printing the terminals, then nonterminals of grammar in appearing order
 * output is one line, and all names are space delineated
 */
void Task1()
{
    vector<string> terminals;
    terminals.reserve(symbol_order.size());
    for (const auto &name : symbol_order)
    {
        if (nonterminal_symbols.find(name) == nonterminal_symbols.end())
        {
            terminals.push_back(name);
        }
    }

    bool first = true;
    auto emit_list = [&](const vector<string> &symbols) {
        for (const auto &name : symbols)
        {
            if (!first)
            {
                cout << ' ';
            }
            cout << name;
            first = false;
        }
    };

    // emit the concatenated terminal, nonterminal listings on one physical line
    emit_list(terminals);
    emit_list(nonterminal_order);
    cout << '\n';
}

/*
 * Task 2:
 * Print out nullable set of the grammar in specified format.
 */

static unordered_map<string, bool> ComputeNullable()
{
    unordered_map<string, bool> nullable;
    for (const auto &name : nonterminal_order)
    {
        nullable[name] = false;
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const auto &rule : grammar_rules)
        {
            bool rhs_nullable = true;
            if (rule.rhs.empty())
            {
                rhs_nullable = true;
            }
            else
            {
                for (const auto &symbol : rule.rhs)
                {
                    if (!IsNonterminal(symbol) || !nullable[symbol])
                    {
                        rhs_nullable = false;
                        break;
                    }
                }
            }

            if (rhs_nullable && !nullable[rule.lhs])
            {
                // mark new nullable head; continue until a full pass finds no updates
                nullable[rule.lhs] = true;
                changed = true;
            }
        }
    }

    return nullable;
}

static bool IsNullableSymbol(const string &name, const unordered_map<string, bool> &nullable)
{
    const auto it = nullable.find(name);
    return it != nullable.end() && it->second;
}

static bool InsertAll(unordered_set<string> &destination, const unordered_set<string> &source)
{
    bool changed = false;
    for (const auto &item : source)
    {
        if (destination.insert(item).second)
        {
            changed = true;
        }
    }
    return changed;
}

static bool InsertAllExceptEpsilon(unordered_set<string> &destination,
                                   const unordered_set<string> &source)
{
    bool changed = false;
    for (const auto &item : source)
    {
        if (item == "epsilon")
        {
            continue;
        }
        if (destination.insert(item).second)
        {
            changed = true;
        }
    }
    return changed;
}

static unordered_map<string, unordered_set<string>> ComputeFirstSets(const unordered_map<string, bool> &nullable)
{
    unordered_map<string, unordered_set<string>> first_sets;

    for (const auto &symbol : symbol_order)
    {
        if (!IsNonterminal(symbol))
        {
            first_sets[symbol].insert(symbol);
        }
        else
        {
            first_sets[symbol];
        }
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const auto &rule : grammar_rules)
        {
            auto &lhs_first = first_sets[rule.lhs];

            if (rule.rhs.empty())
            {
                if (lhs_first.insert("epsilon").second)
                {
                    changed = true;
                }
                continue;
            }

            bool prefix_nullable = true;
            for (const auto &symbol : rule.rhs)
            {
                auto &symbol_first = first_sets[symbol];
                if (InsertAllExceptEpsilon(lhs_first, symbol_first))
                {
                    changed = true;
                }

                if (!IsNullableSymbol(symbol, nullable))
                {
                    // as soon as a symbol is not nullable, the prefix cannot leak epsilon further
                    prefix_nullable = false;
                    break;
                }
            }

            if (prefix_nullable)
            {
                if (lhs_first.insert("epsilon").second)
                {
                    changed = true;
                }
            }
        }
    }

    return first_sets;
}

static void PrintFirstSet(const unordered_set<string> &first_set)
{
    cout << "{ ";
    bool first = true;
    for (const auto &symbol : symbol_order)
    {
        if (first_set.find(symbol) != first_set.end())
        {
            if (!first)
            {
                cout << ", ";
            }
            cout << symbol;
            first = false;
        }
    }
    cout << " }";
}

static void PrintFollowSet(const unordered_set<string> &follow_set)
{
    cout << "{ ";
    bool printed = false;
    if (follow_set.find("$") != follow_set.end())
    {
        cout << "$";
        printed = true;
    }

    for (const auto &symbol : symbol_order)
    {
        if (follow_set.find(symbol) != follow_set.end())
        {
            if (printed)
            {
                cout << ", ";
            }
            cout << symbol;
            printed = true;
        }
    }

    cout << " }";
}

static size_t CommonPrefixLength(const vector<string> &a, const vector<string> &b)
{
    const size_t limit = min(a.size(), b.size());
    size_t length = 0;
    while (length < limit && a[length] == b[length])
    {
        ++length;
    }
    return length;
}

static bool HasPrefix(const vector<string> &sequence, const vector<string> &prefix)
{
    if (sequence.size() < prefix.size())
    {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); ++i)
    {
        if (sequence[i] != prefix[i])
        {
            return false;
        }
    }
    return true;
}

static bool SequenceLexLess(const vector<string> &lhs, const vector<string> &rhs)
{
    const size_t limit = min(lhs.size(), rhs.size());
    for (size_t i = 0; i < limit; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return lhs[i] < rhs[i];
        }
    }
    return lhs.size() < rhs.size();
}

static bool RuleLexLess(const Rule &a, const Rule &b)
{
    if (a.lhs != b.lhs)
    {
        return a.lhs < b.lhs;
    }
    const size_t limit = min(a.rhs.size(), b.rhs.size());
    for (size_t i = 0; i < limit; ++i)
    {
        if (a.rhs[i] != b.rhs[i])
        {
            return a.rhs[i] < b.rhs[i];
        }
    }
    return a.rhs.size() < b.rhs.size();
}

static pair<size_t, vector<string>> FindBestPrefix(const vector<Rule> &rules)
{
    size_t best_length = 0;
    vector<string> best_prefix;
    const size_t count = rules.size();

    for (size_t i = 0; i < count; ++i)
    {
        for (size_t j = i + 1; j < count; ++j)
        {
            const size_t length = CommonPrefixLength(rules[i].rhs, rules[j].rhs);
            if (length == 0)
            {
                continue;
            }

            vector<string> prefix(rules[i].rhs.begin(), rules[i].rhs.begin() + length);

            if (length > best_length || (length == best_length && SequenceLexLess(prefix, best_prefix)))
            {
                best_length = length;
                best_prefix = std::move(prefix);
            }
        }
    }

    return {best_length, best_prefix};
}

static vector<string> SortedNonterminals(const unordered_map<string, vector<Rule>> &rules_by_nt)
{
    vector<string> nts;
    nts.reserve(rules_by_nt.size());
    for (const auto &entry : rules_by_nt)
    {
        nts.push_back(entry.first);
    }
    sort(nts.begin(), nts.end());
    return nts;
}

void Task2()
{
    auto nullable = ComputeNullable();

        cout << "Nullable = { ";
    bool first = true;
    for (const auto &nt : nonterminal_order)
    {
        if (nullable[nt])
        {
            if (!first)
            {
                cout << ", ";
            }
            cout << nt;
            first = false;
        }
    }
    cout << " }\n";
}

// Task 3: FIRST sets
void Task3()
{
    // FIRST requires nullable info to decide when to keep walking the RHS
    auto nullable = ComputeNullable();
    auto first_sets = ComputeFirstSets(nullable);

    for (const auto &nt : nonterminal_order)
    {
        cout << "FIRST(" << nt << ") = ";
        PrintFirstSet(first_sets[nt]);
        cout << '\n';
    }
}

// Task 4: FOLLOW sets
void Task4()
{
    // FOLLOW uses both nullable and precomputed FIRST sets for suffix analysis
    auto nullable = ComputeNullable();
    auto first_sets = ComputeFirstSets(nullable);

    unordered_map<string, unordered_set<string>> follow_sets;
    for (const auto &nt : nonterminal_order)
    {
        follow_sets[nt];
    }

    if (!start_symbol.empty())
    {
        follow_sets[start_symbol].insert("$");
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const auto &rule : grammar_rules)
        {
            const size_t rhs_size = rule.rhs.size();

            for (size_t i = 0; i < rhs_size; ++i)
            {
                const string &symbol = rule.rhs[i];
                if (!IsNonterminal(symbol))
                {
                    continue;
                }

                unordered_set<string> beta_first;
                bool beta_nullable = true;

                for (size_t j = i + 1; j < rhs_size; ++j)
                {
                    const string &next_symbol = rule.rhs[j];
                    const auto &next_first = first_sets[next_symbol];
                    for (const auto &entry : next_first)
                    {
                        if (entry != "epsilon")
                        {
                            beta_first.insert(entry);
                        }
                    }

                    if (!IsNullableSymbol(next_symbol, nullable))
                    {
                        beta_nullable = false;
                        break;
                    }
                }

                if (InsertAll(follow_sets[symbol], beta_first))
                {
                    changed = true;
                }

        if (beta_nullable)
        {
            if (InsertAll(follow_sets[symbol], follow_sets[rule.lhs]))
            {
                changed = true;
            }
        }
    }
        }
    }

    for (const auto &nt : nonterminal_order)
    {
        cout << "FOLLOW(" << nt << ") = ";
        PrintFollowSet(follow_sets[nt]);
        cout << '\n';
    }
}

// Task 5: left factoring
void Task5()
{
    // stage every rule under its LHS so we can repeatedly factor one nonterminal at a time
    unordered_map<string, vector<Rule>> pending;
    for (const auto &rule : grammar_rules)
    {
        pending[rule.lhs].push_back(rule);
    }

    deque<string> worklist;
    for (const auto &nt : nonterminal_order)
    {
        if (pending.find(nt) != pending.end())
        {
            worklist.push_back(nt);
        }
    }

    unordered_map<string, int> name_counters;
    vector<Rule> result;

    while (!worklist.empty())
    {
        const string current = worklist.front();
        worklist.pop_front();

        auto pending_it = pending.find(current);
        if (pending_it == pending.end())
        {
            continue;
        }

        auto &rules = pending_it->second;
        auto best = FindBestPrefix(rules);
        const size_t prefix_length = best.first;

        if (prefix_length == 0)
        {
            // nothing left to factor for this nonterminal; move rules to the output pool
            result.insert(result.end(), rules.begin(), rules.end());
            pending.erase(pending_it);
            continue;
        }

        const vector<string> &prefix = best.second;
        string new_name = current + to_string(++name_counters[current]);

        vector<Rule> updated_rules;
        vector<Rule> new_name_rules;
        updated_rules.reserve(rules.size());
        new_name_rules.reserve(rules.size());

        for (const auto &rule : rules)
        {
            if (HasPrefix(rule.rhs, prefix))
            {
                // split off suffixes feeding the freshly minted helper nonterminal
                vector<string> suffix(rule.rhs.begin() + prefix_length, rule.rhs.end());
                new_name_rules.push_back({new_name, std::move(suffix)});
            }
            else
            {
                updated_rules.push_back(rule);
            }
        }

        vector<string> new_rhs = prefix;
        new_rhs.push_back(new_name);
        updated_rules.push_back({current, std::move(new_rhs)});

        rules = std::move(updated_rules);
        result.insert(result.end(), new_name_rules.begin(), new_name_rules.end());

        // re-queue the nonterminal since its rule set changed and may still share prefixes
        worklist.push_front(current);
    }

    sort(result.begin(), result.end(), RuleLexLess);

    for (const auto &rule : result)
    {
        cout << rule.lhs << " -> ";
        if (rule.rhs.empty())
        {
            cout << "#\n";
        }
        else
        {
            for (size_t i = 0; i < rule.rhs.size(); ++i)
            {
                if (i > 0)
                {
                    cout << ' ';
                }
                cout << rule.rhs[i];
            }
            cout << " #\n";
        }
    }
}

// Task 6: eliminate left recursion
void Task6()
{
    // duplicate rules bucketed by LHS so transformations do not mutate the original vector
    unordered_map<string, vector<Rule>> rules_by_nt;
    for (const auto &rule : grammar_rules)
    {
        rules_by_nt[rule.lhs].push_back(rule);
    }

    vector<string> sorted_nts = SortedNonterminals(rules_by_nt);

    unordered_map<string, vector<Rule>> processed_rules;
    unordered_map<string, int> name_counters;

    for (size_t i = 0; i < sorted_nts.size(); ++i)
    {
        const string &Ai = sorted_nts[i];
        auto &Ai_rules = rules_by_nt[Ai];

        for (size_t j = 0; j < i; ++j)
        {
            const string &Aj = sorted_nts[j];
            vector<Rule> updated;

            for (auto &rule : Ai_rules)
            {
                if (!rule.rhs.empty() && rule.rhs[0] == Aj)
                {
                    // substitute Aj's current productions to eliminate indirect left recursion
                    vector<Rule> replacements;
                    if (processed_rules.count(Aj))
                    {
                        replacements = processed_rules[Aj];
                    }
                    else
                    {
                        replacements = rules_by_nt[Aj];
                    }

                    for (const auto &rep : replacements)
                    {
                        vector<string> new_rhs = rep.rhs;
                        new_rhs.insert(new_rhs.end(), rule.rhs.begin() + 1, rule.rhs.end());
                        updated.push_back({Ai, std::move(new_rhs)});
                    }
                }
                else
                {
                    updated.push_back(rule);
                }
            }

            Ai_rules = std::move(updated);
        }

        vector<Rule> alpha;
        vector<Rule> beta;
        for (const auto &rule : Ai_rules)
        {
            if (!rule.rhs.empty() && rule.rhs[0] == Ai)
            {
                // these are the α pieces (immediate left recursion)
                vector<string> suffix(rule.rhs.begin() + 1, rule.rhs.end());
                alpha.push_back({Ai, std::move(suffix)});
            }
            else
            {
                // β productions can stay but will point at the new helper if α exists
                beta.push_back(rule);
            }
        }

        if (alpha.empty())
        {
            processed_rules[Ai] = Ai_rules;
        }
        else
        {
            // roll standard elimination pattern: Ai -> β Ai', Ai' -> α Ai' | ε
            string new_name = Ai + to_string(++name_counters[Ai]);

            vector<Rule> new_Ai_rules;
            for (auto &rule : beta)
            {
                rule.rhs.push_back(new_name);
                new_Ai_rules.push_back(std::move(rule));
            }

            vector<Rule> new_name_rules;
            new_name_rules.push_back({new_name, {}});
            for (const auto &rule : alpha)
            {
                vector<string> rhs = rule.rhs;
                rhs.push_back(new_name);
                new_name_rules.push_back({new_name, std::move(rhs)});
            }

            processed_rules[Ai] = std::move(new_Ai_rules);
            processed_rules[new_name] = std::move(new_name_rules);
        }

        rules_by_nt[Ai] = processed_rules[Ai];
    }

    vector<Rule> final_rules;
    for (const auto &entry : processed_rules)
    {
        final_rules.insert(final_rules.end(), entry.second.begin(), entry.second.end());
    }

    sort(final_rules.begin(), final_rules.end(), RuleLexLess);

    for (const auto &rule : final_rules)
    {
        cout << rule.lhs << " -> ";
        if (rule.rhs.empty())
        {
            cout << "#\n";
        }
        else
        {
            for (size_t i = 0; i < rule.rhs.size(); ++i)
            {
                if (i > 0)
                {
                    cout << ' ';
                }
                cout << rule.rhs[i];
            }
            cout << " #\n";
        }
    }
}

int main(int argc, char *argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);

    ReadGrammar(); // Reads the input grammar from standard input
                   // and represent it internally in data structures
                   // ad described in project 2 presentation file

    switch (task)
    {
    case 1:
        Task1();
        break;

    case 2:
        Task2();
        break;

    case 3:
        Task3();
        break;

    case 4:
        Task4();
        break;

    case 5:
        Task5();
        break;

    case 6:
        Task6();
        break;

    default:
        cout << "Error: unrecognized task number " << task << "\n";
        break;
    }
    return 0;
}
