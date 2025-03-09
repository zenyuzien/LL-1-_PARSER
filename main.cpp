
/*
 * @author Rushikesh Muraharisetty
 * @file main.cpp
 * @brief CIA-2 - advanced compiler design - ll(1) parser
 *
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define debug 0

std::unordered_map<char, int> indexOf;
std::unordered_map<int, char> terminalOf, nonTerminalOf;
std::unordered_set<char> terminals;
std::unordered_set<char> nonTerminals;
std::vector<std::vector<std::string> > record;
char epsilon_symbol;
char int_symbol;
char eof_symbol;
std::vector<char> secondLineChars;
std::vector<char> fourthLineChars;
std::vector<std::vector<std::pair<std::vector<std::string>, char> > > helper;
std::vector<std::vector<int> > firsts;

struct stack // not optimal but suffices just for this use case so okay
{
    std::vector<std::pair<int, bool> > v;
    int max_index, cur_index;

    void
    init ()
    {
        max_index = cur_index = 0;
        v.push_back ({ indexOf[eof_symbol], 1 });
    }

    void
    push (std::pair<int, bool> ele)
    {
        if (debug)
            {
                std::cout << "pushed ";
                if (ele.second)
                    std::cout << terminalOf[ele.first] << std::endl;
                else
                    std::cout << nonTerminalOf[ele.first] << std::endl;
            }
        if (cur_index < max_index)
            {
                v[++cur_index] = ele;
            }
        else if (cur_index == max_index)
            {
                v.push_back (ele);
                cur_index = (++max_index);
            }
    }
    std::pair<int, bool>
    pop ()
    {
        if (cur_index >= 0)
            {
                if (debug)
                    {
                        std::cout << "popping ";
                        if (v[cur_index].second)
                            std::cout << terminalOf[v[cur_index].first] << std::endl;
                        else
                            std::cout << nonTerminalOf[v[cur_index].first] << std::endl;
                    }
                return v[cur_index--];
            }

        std::cout << "error popping: empty ! \n";
        return { -1, 0 };
    }

    void
    print ()
    {
        for (int i = 0; i <= cur_index; i++)
            {
                if (v[i].second)
                    std::cout << terminalOf[v[i].first] << " ";
                else
                    std::cout << nonTerminalOf[v[i].first] << " ";
            }
        std::cout << "\n";
    }
};

bool
isTerminal (char c)
{
    return terminals.find (c) != terminals.end ();
}

bool
isNonTerminal (char c)
{
    return nonTerminals.find (c) != nonTerminals.end ();
}

void
validateAndInsert (std::vector<char> &vec, const std::string &line, int lineNumber, bool isTerminalSet)
{
    std::istringstream iss (line);
    std::string token;
    while (iss >> token)
        {
            if (token.length () != 1)
                {
                    std::cerr << "Validation failed at line " << lineNumber << ": '" << token
                              << "' is not a single character.\n";
                    exit (EXIT_FAILURE);
                }
            char ch = token[0];
            if (isTerminalSet)
                {
                    indexOf[ch] = terminals.size ();
                    terminalOf[indexOf[ch]] = ch;
                    terminals.insert (ch);
                }
            else
                {
                    indexOf[ch] = nonTerminals.size ();
                    nonTerminalOf[indexOf[ch]] = ch;
                    nonTerminals.insert (ch);
                }
            vec.push_back (ch);
        }
}

bool
hasDuplicate (const std::vector<char> &vec1, const std::vector<char> &vec2)
{
    std::unordered_set<char> seen;

    for (char c : vec1)
        {
            if (!seen.insert (c).second)
                {
                    std::cout << "Duplicate terminal '" << c << "' found.\n";
                    return true;
                }
        }
    for (char c : vec2)
        {
            if (!seen.insert (c).second)
                {
                    std::cout << "Duplicate symbol '" << c << "' found in nonTerminals.\n";
                    return true;
                }
        }
    return false;
}

int
init ()
{
    std::ifstream file ("input.txt");
    if (!file)
        {
            std::cerr << "Error opening file!\n";
            exit (EXIT_FAILURE);
        }

    std::string line;
    int lineNumber = 0;

    while (std::getline (file, line))
        {
            lineNumber++;
            if (lineNumber == 2)
                {
                    validateAndInsert (secondLineChars, line, lineNumber, true);
                }
            else if (lineNumber == 4)
                {
                    validateAndInsert (fourthLineChars, line, lineNumber, false);
                }
        }

    if (terminals.size () == 0 || nonTerminals.size () == 0)
        {
            file.close ();
            std::cerr << "no symbols !";
            return 0;
        }

    epsilon_symbol = secondLineChars[0];
    int_symbol = secondLineChars[1];
    eof_symbol = secondLineChars[terminals.size () - 1];

    file.clear ();
    file.seekg (0, std::ios::beg);
    lineNumber = 0;
    record.resize (nonTerminals.size ());

    while (std::getline (file, line))
        {
            lineNumber++;
            if (lineNumber >= 5 && line.find (">") != std::string::npos)
                {
                    char nonTerminal = line[0];
                    if (isNonTerminal (nonTerminal))
                        {
                            std::string production = line.substr (2);
                            std::istringstream iss (production);
                            std::string part;
                            while (std::getline (iss, part, '|'))
                                {
                                    record[indexOf[nonTerminal]].push_back (part);
                                }
                        }
                }
        }

    file.close ();

    if (hasDuplicate (secondLineChars, fourthLineChars))
        return 0;

    std::cout << "\n\nterminals: ";
    for (char c : secondLineChars)
        {
            std::cout << c << " ";
        }
    std::cout << "\nwhere epsilon is " << epsilon_symbol << " and int symbol is " << int_symbol << "\n";

    std::cout << "non terminals: ";
    for (char c : fourthLineChars)
        {
            std::cout << c << " ";
        }
    std::cout << "\n";

    /* std::cout << "Index mapping of symbols:\n";
     for (const auto& pair : indexOf) {
         std::cout << pair.first << " -> " << pair.second << "\n";
     }*/

    std::cout << "\nProduction Rules:\n";
    int idx = 0;
    for (char nt : fourthLineChars)
        {
            std::cout << nt << " -> ";
            for (size_t j = 0; j < record[idx].size (); j++)
                {
                    std::cout << record[idx][j];
                    if (j < record[idx].size () - 1)
                        std::cout << " | ";
                }
            std::cout << "\n";
            idx++;
        }
    return 1;
}

std::vector<int>
convertToIndices (const std::string &expression)
{
    std::vector<int> result;
    size_t i = 0;

    while (i < expression.size ())
        {
            char c = expression[i];

            if (isdigit (c))
                {
                    while (i < expression.size () && isdigit (expression[i]))
                        {
                            i++;
                        }
                    result.push_back (indexOf.at (int_symbol));
                }
            else if (indexOf.find (c) != indexOf.end ())
                {
                    result.push_back (indexOf.at (c));
                    i++;
                }
            else
                {
                    std::cerr << "Error: Unrecognized character '" << c << "' in expression.\n";
                    exit (EXIT_FAILURE);
                }
        }

    return result;
}

std::vector<int>
FIRST (int nonTerminalIndex)
{
    std::vector<int> firstSet;

    for (const std::string &production : record[nonTerminalIndex])
        {
            int i = 0;
            bool foundTerminal = false;

            while (i < production.size ())
                {
                    char symbol = production[i];

                    if (isTerminal (symbol))
                        {
                            firstSet.push_back (indexOf[symbol]);
                            foundTerminal = true;
                            break;
                        }
                    else if (isNonTerminal (symbol))
                        {
                            std::vector<int> subFirst = FIRST (indexOf[symbol]);

                            bool hasEpsilon = false;
                            for (int val : subFirst)
                                {
                                    if (val == indexOf[epsilon_symbol])
                                        {
                                            hasEpsilon = true; 
                                        }
                                    else
                                        {
                                            firstSet.push_back (val); 
                                        }
                                }

                            if (!hasEpsilon)
                                {
                                    foundTerminal = true;
                                    break; 
                                }
                        }
                    else
                        {
                            std::cerr << "Error: Unknown symbol in production: " << symbol << std::endl;
                            exit (EXIT_FAILURE);
                        }

                    i++;
                }

            if (!foundTerminal)
                {
                    firstSet.push_back (indexOf[epsilon_symbol]);
                }
        }

    std::sort (firstSet.begin (), firstSet.end ());
    firstSet.erase (std::unique (firstSet.begin (), firstSet.end ()),
                    firstSet.end ());

    return firstSet;
}

void
constructHelper ()
{
    helper.resize (nonTerminals.size ());
    for (size_t i = 0; i < record.size (); i++)
        {
            char nonTerminal = nonTerminalOf[i];

            for (const std::string &production : record[i])
                {
                    for (size_t j = 0; j < production.size (); j++)
                        {
                            char symbol = production[j];

                            if (isNonTerminal (symbol))
                                {
                                    helper[indexOf[symbol]].push_back ({ { production }, nonTerminal });
                                }
                        }
                }
        }

    std::cout << "\nHelper:\n";
    for (size_t i = 0; i < nonTerminals.size (); i++)
        {
            char nt = nonTerminalOf[i];
            std::cout << "helper[" << nt << "] = { ";
            for (const auto &entry : helper[i])
                {
                    std::cout << "{ ";
                    for (const std::string &prod : entry.first)
                        {
                            std::cout << "\"" << prod << "\" ";
                        }
                    std::cout << ", " << entry.second << " } ";
                }
            std::cout << "}\n";
        }
}
std::vector<std::set<int> > holder;

std::vector<std::unordered_set<int> > required;
bool
allRequiredEmpty ()
{
    for (const auto &reqSet : required)
        {
            if (!reqSet.empty ())
                {
                    return false; 
                }
        }
    return true;
}
std::vector<std::unordered_set<int> > follows;
void
printdep ()
{
    for (size_t i = 0; i < required.size (); ++i)
        {
            std::cout << "dep[" << nonTerminalOf[i] << "]: { ";
            for (auto j : required[i])
                {
                    std::cout << nonTerminalOf[j] << " ";
                }
            std::cout << "}" << std::endl;
        }
}
void
FOLLOW ()
{
    std::vector<bool> visit;
    visit.resize (nonTerminals.size (), 0);
    visit[0] = true;

    required.resize (nonTerminals.size ());

    follows.resize (nonTerminals.size ());
    follows[0] = { indexOf[eof_symbol] };

    for (int i = 0; i < nonTerminals.size (); ++i)
        {
            if (debug)
                std::cout << "\nProcessing Non-Terminal: " << nonTerminalOf[i] << "\n";

            for (auto pairs : helper[i])
                {
                    if (debug)
                        {
                            std::cout << "  Rule: " << pairs.second << " -> ";
                            for (auto s : pairs.first)
                                std::cout << s;
                            std::cout << "\n";
                        }

                    for (auto str : pairs.first)
                        {
                            bool activate = false;

                            for (auto c : str)
                                {
                                    if (debug)
                                        {
                                            std::cout << "    In string, checking: " << c << "\n";
                                        }

                                    if (!activate)
                                        {
                                            if (c == nonTerminalOf[i])
                                                {
                                                    activate = true;
                                                    if (debug)
                                                        std::cout << "    -> Activated: Found " << c << " in rule\n";
                                                }
                                            continue;
                                        }

                                    // Activated, now handle cases
                                    if (isTerminal (c))
                                        {
                                            follows[i].insert (indexOf[c]);
                                            if (debug)
                                                std::cout << "    -> Case 1: Terminal found! Adding " << c << " (index "
                                                          << indexOf[c] << ") to follows[" << nonTerminalOf[i] << "]\n";
                                            if (debug)
                                                {
                                                    std::cout << "After Insertion (Follow[" << nonTerminalOf[i]
                                                              << "]): ";
                                                    for (int val : follows[i])
                                                        std::cout << terminalOf[val] << " ";
                                                    std::cout << "\n";
                                                }
                                            activate = false;
                                        }
                                    else if (isNonTerminal (c))
                                        {
                                            bool isEpsilon = false;
                                            if (debug)
                                                std::cout << "    -> Case 3: Non-Terminal found! Checking first(" << c
                                                          << ")\n";

                                            for (auto p : firsts[indexOf[c]])
                                                {
                                                    if (p == indexOf[epsilon_symbol])
                                                        {
                                                            isEpsilon = true;
                                                            if (debug)
                                                                std::cout << "      - Epsilon found in first(" << c
                                                                          << "), continue searching\n";
                                                        }
                                                    else
                                                        {
                                                            follows[i].insert (p);
                                                            if (debug)
                                                                std::cout << "      - Adding first(" << c << ") entry "
                                                                          << p << " to follows[" << nonTerminalOf[i]
                                                                          << "]\n";
                                                        }
                                                }

                                            if (!isEpsilon)
                                                {
                                                    activate = false;
                                                    if (debug)
                                                        std::cout << "    -> No epsilon, deactivating\n";
                                                }
                                        }
                                    else
                                        {
                                            if (debug)
                                                std::cout
                                                    << "    -> Impossible case! Unexpected character encountered.\n";
                                        }
                                }

                            if (activate)
                                {
                                    if (debug)
                                        {
                                            std::cout << "before Insertion (Follow[" << nonTerminalOf[i] << "]): ";
                                            for (int val : follows[i])
                                                std::cout << terminalOf[val] << " ";
                                            std::cout << "\n";
                                        }

                                    follows[i].insert (follows[indexOf[pairs.second]].begin (),
                                                       follows[indexOf[pairs.second]].end ());

                                    if (debug)
                                        std::cout << "    -> End of rule: Adding follows[" << pairs.second
                                                  << "] into follows[" << nonTerminalOf[i] << "]\n";
                                    if (debug)
                                        {
                                            std::cout << "After Insertion (Follow[" << nonTerminalOf[i] << "]): ";
                                            for (int val : follows[i])
                                                std::cout << terminalOf[val] << " ";
                                            std::cout << "\n";
                                        }

                                    if (!visit[indexOf[pairs.second]])
                                        {

                                            if (i != indexOf[pairs.second])
                                                required[i].insert (indexOf[pairs.second]);
                                            if (debug)
                                                std::cout << "    -> Not done yet! " << nonTerminalOf[i]
                                                          << " requires follow of " << pairs.second << "\n";
                                        }
                                    else
                                        {
                                            if (debug)
                                                std::cout << " appending dependencies !  ";
                                            required[i].insert (required[indexOf[pairs.second]].begin (),
                                                                required[indexOf[pairs.second]].end ());
                                            required[i].erase (i);
                                            if (debug)
                                                {
                                                    std::cout << "After Insertion (req[" << nonTerminalOf[i] << "]): ";
                                                    for (int val : required[i])
                                                        std::cout << nonTerminalOf[val] << " ";
                                                    std::cout << "\n";
                                                };
                                        }
                                }
                        }
                }

            visit[i] = true;
        }
    // printdep();
    while (!allRequiredEmpty ())
        {
            for (int i = 1; i < nonTerminals.size (); i++)
                {
                    for (auto it = required[i].begin (); it != required[i].end ();)
                        {
                            if (required[*it].empty ())
                                {
                                    follows[i].insert (follows[*it].begin (), follows[*it].end ());
                                    it = required[i].erase (it); 
                                }
                            else
                                {
                                    ++it; 
                                }
                        }
                }
        }

    // printdep();
}
void
printFollows ()
{
    for (size_t i = 0; i < follows.size (); ++i)
        {
            std::cout << "Follow[" << nonTerminalOf[i] << "]: { ";
            for (auto j : follows[i])
                {
                    std::cout << terminalOf[j] << " ";
                }
            std::cout << "}" << std::endl;
        }
    std::cout << "\n";
}

std::vector<std::vector<std::vector<std::pair<int, bool> > > > parse_table;

std::vector<std::pair<int, bool> >
getvectorfromstr (std::string str)
{
    std::vector<std::pair<int, bool> > res; // 1 if terminal , 0 if non terminal
    for (auto c : str)
        {
            // std::cout<<"in "<< c <<" "<< indexOf[c]<<" "<< (bool)isTerminal(c) << " :: ";
            res.push_back ({ indexOf[c], (bool)isTerminal (c) });
        } // std::cout<<"\n";

    return res;
}

void
init_parse_table ()
{
    parse_table.resize (nonTerminals.size ());
    for (int i = 0; i < nonTerminals.size (); ++i)
        parse_table[i].resize (terminals.size ());

    int i = 0;
    for (const auto &vstr : record)
        {
            for (const auto &str : vstr)
                {
                    if (debug)
                        std::cout << "in " << str << " --> \n";
                    auto entry = getvectorfromstr (str);

                    // case 2 e terminal
                    if (str[0] == epsilon_symbol)
                        {
                            for (auto ei : follows[i])
                                {
                                    parse_table[i][ei] = entry;
                                    if (debug)
                                        std::cout << "\tP[" << nonTerminalOf[i] << "][" << terminalOf[ei]
                                                  << "] =" << str << "\n";
                                }
                        }
                    else
                        // case 1 non-e terminal
                        if (isTerminal (str[0]))
                            {
                                parse_table[i][indexOf[str[0]]] = entry;
                                if (debug)
                                    std::cout << "\tP[" << nonTerminalOf[i] << "][" << terminalOf[indexOf[str[0]]]
                                              << "] =" << str << "\n";
                            }

                        else
                            {
                                // case 4 non terminal with e
                                if (follows[indexOf[str[0]]].find (0) != follows[indexOf[str[0]]].end ())
                                    {
                                        for (auto ei : firsts[indexOf[str[0]]])
                                            {
                                                parse_table[i][ei] = entry;
                                                if (debug)
                                                    std::cout << "\tP[" << nonTerminalOf[i] << "][" << terminalOf[ei]
                                                              << "] =" << str << "\n";
                                            }
                                    }
                                else
                                    { // case 3 Non terminal without e
                                        for (auto ei : firsts[indexOf[str[0]]])
                                            {
                                                parse_table[i][ei] = entry;
                                                if (debug)
                                                    std::cout << "\tP[" << nonTerminalOf[i] << "][" << terminalOf[ei]
                                                              << "] =" << str << "\n";
                                            }
                                    }
                            }
                }
            i++;
        }
}

void
printParseTable ()
{
    for (size_t i = 0; i < parse_table.size (); ++i)
        {
            std::cout << "Row " << nonTerminalOf[i] << ":\n";
            for (size_t j = 0; j < parse_table[i].size (); ++j)
                {
                    std::cout << "  Column " << terminalOf[j] << ": ";
                    if (parse_table[i][j].empty ())
                        {
                            std::cout << "-"; // Empty cell representation
                        }
                    else
                        {
                            for (size_t k = 0; k < parse_table[i][j].size (); ++k)
                                {
                                    if (parse_table[i][j][k].second)
                                        std::cout << terminalOf[parse_table[i][j][k].first];
                                    else
                                        std::cout << nonTerminalOf[parse_table[i][j][k].first];
                                }
                        }
                    std::cout << "\n";
                }
            std::cout << "---------------------------------\n";
        }
}

void
printRecord ()
{
    int i = 0;
    for (const auto &row : record)
        { 
            std::cout << nonTerminalOf[i++] << ": ";
            for (const auto &value : row)
                {                              
                    std::cout << value << " "; 
                }
            std::cout << std::endl;
        }
    std::cout << std::endl;
}

bool
eval (std::string &str)
{
    int str_len = str.length ();
    struct stack stack;
    stack.init ();
    stack.push ({ 0, 0 });

    int ptr = 0;

    while (stack.cur_index >= 0)
        {
            if (ptr >= str_len)
                {
                    std::cout << "unexpected EOF, parse error \n";
                    return false;
                }

            if (debug)
                std::cout << "currently in " << str[ptr] << "\n";
            auto ele = stack.pop ();
            if (debug)
                std::cout << "stack: ";
            if (debug)
                stack.print ();

            if (ele.second == 1)
                {
                    if (ele.first == indexOf[str[ptr]])
                        {
                            if (debug)
                                std::cout << "stack top and pointer match! \n";
                            ptr++;
                            continue;
                        }
                    else
                        {
                            std::cout << "mismatched terminals, parse fail. expected: " << terminalOf[ele.first]
                                      << std::endl;
                            return false;
                        }
                }
            else
                {
                    auto entry = parse_table[ele.first][indexOf[str[ptr]]];
                    if (debug)
                        std::cout << "checking p[" << nonTerminalOf[ele.first] << "][" << terminalOf[indexOf[str[ptr]]]
                                  << "]  size " << entry.size () << "\n";

                    if (entry.size () == 0)
                        {
                            std::cout << "no entry, parse error \n";
                            return false;
                        }
                    std::reverse (entry.begin (), entry.end ());
                    if (debug)
                        std::cout << "pushing ";
                    for (auto c : entry)
                        {
                            if (debug)
                                if (c.second)
                                    std::cout << terminalOf[c.first] << " ";
                                else
                                    std::cout << nonTerminalOf[c.first] << " ";

                            if ((c.second == 1) && (c.first == 0))
                                {
                                    if (debug)
                                        std::cout << " epsilon wont be pushed ! \n";
                                    continue;
                                }
                            stack.push (c);
                        }
                    if (debug)
                        std::cout << "\n";
                }
        }
    if (ptr < str_len)
        {
            std::cout << "string remaining: Unexpected char, parse error \n";
            return false;
        }

    if (debug)
        std::cout << "parse success ! \n";
    return true;
}

int
main ()
{
    if (!init ())
        return 0;

    std::string expr = "i*i+i";

    expr += "$";
    std::vector<int> indices = convertToIndices (expr);

    std::cout << "\nThe input expression in token form : ";
    for (int idx : indices)
        std::cout << terminalOf[idx] << " ";

    std::cout << "\n\n";
    firsts.resize (fourthLineChars.size ());
    for (auto c : fourthLineChars)
        {
            std::cout << "first set of " << c << " is ";
            firsts[indexOf[c]] = FIRST (indexOf[c]);
            for (int i : firsts[indexOf[c]])
                std::cout << terminalOf[i] << " ";
            std::cout << "\n";
        }
    std::cout << "\n\n";
    constructHelper ();
    std::cout << "\n\n";
    FOLLOW ();
    printFollows ();

    printRecord ();
    init_parse_table ();
    printParseTable ();

    if (eval (expr))
        std::cout << "parsing success ! \n";

    return 0;
}
