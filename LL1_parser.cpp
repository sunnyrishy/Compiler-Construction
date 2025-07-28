#include <bits/stdc++.h>
#include <fstream>
using namespace std;

struct Production
{
    string left;
    string right;
    int index;

    Production(string l, string r, int idx = -1) : left(l), right(r), index(idx) {}
};

class GrammarConverter
{
private:
    vector<Production> productions;
    set<string> nonTerminals;
    set<string> terminals;
    map<string, set<string>> firstSets;
    map<string, set<string>> followSets;
    map<pair<string, string>, int> parseTable; // [nonTerminal][terminal] -> production index

    void parseProduction(string input)
    {
        size_t arrowPos = input.find("->");
        if (arrowPos == string::npos)
            return;

        string left = input.substr(0, arrowPos);
        string right = input.substr(arrowPos + 2);

        // Trim whitespace
        left.erase(0, left.find_first_not_of(" \t"));
        left.erase(left.find_last_not_of(" \t") + 1);
        right.erase(0, right.find_first_not_of(" \t"));
        right.erase(right.find_last_not_of(" \t") + 1);

        // Handle special case for parentheses - split (E) into ( E )
        string processedRight = "";
        for (int i = 0; i < right.length(); i++)
        {
            if (right[i] == '(' || right[i] == ')')
            {
                processedRight += " ";
                processedRight += right[i];
                processedRight += " ";
            }
            else
            {
                processedRight += right[i];
            }
        }

        productions.push_back(Production(left, processedRight, productions.size()));
        nonTerminals.insert(left);
    }

    void identifySymbols()
    {
        for (const auto &prod : productions)
        {
            istringstream iss(prod.right);
            string symbol;
            while (iss >> symbol)
            {
                if (nonTerminals.find(symbol) == nonTerminals.end() && symbol != "epsilon")
                {
                    terminals.insert(symbol);
                }
            }
        }
        // Add $ as end marker
        terminals.insert("$");
    }

    bool hasLeftRecursion(const string &nonTerm)
    {
        for (const auto &prod : productions)
        {
            if (prod.left == nonTerm)
            {
                istringstream iss(prod.right);
                string firstSymbol;
                iss >> firstSymbol;
                if (firstSymbol == nonTerm)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void eliminateLeftRecursion()
    {
        vector<Production> newProductions;
        set<string> processedNonTerminals;

        for (const string &A : nonTerminals)
        {
            if (processedNonTerminals.find(A) != processedNonTerminals.end())
            {
                continue;
            }

            vector<string> alpha, beta;

            // Collect alpha (left recursive) and beta (non-left recursive) productions
            for (const auto &prod : productions)
            {
                if (prod.left == A)
                {
                    string processedRight = prod.right;
                    // Clean up extra spaces
                    istringstream iss(processedRight);
                    string cleanRight = "";
                    string word;
                    bool first = true;
                    while (iss >> word)
                    {
                        if (!first)
                            cleanRight += " ";
                        cleanRight += word;
                        first = false;
                    }

                    istringstream iss2(cleanRight);
                    string firstSymbol;
                    iss2 >> firstSymbol;

                    if (firstSymbol == A)
                    {
                        // Left recursive: A -> A alpha
                        string rest = prod.right.substr(firstSymbol.length());
                        rest.erase(0, rest.find_first_not_of(" \t"));
                        alpha.push_back(rest);
                    }
                    else
                    {
                        // Non-left recursive: A -> beta
                        beta.push_back(prod.right);
                    }
                }
            }

            if (!alpha.empty())
            {
                // Has left recursion, need to eliminate
                string newNonTerm = A + "'";
                nonTerminals.insert(newNonTerm);

                // A -> beta A'
                for (const string &b : beta)
                {
                    newProductions.push_back(Production(A, b + " " + newNonTerm, newProductions.size()));
                }

                // A' -> alpha A' | epsilon
                for (const string &a : alpha)
                {
                    newProductions.push_back(Production(newNonTerm, a + " " + newNonTerm, newProductions.size()));
                }
                newProductions.push_back(Production(newNonTerm, "epsilon", newProductions.size()));
            }
            else
            {
                // No left recursion, keep original productions
                for (const auto &prod : productions)
                {
                    if (prod.left == A)
                    {
                        newProductions.push_back(Production(prod.left, prod.right, newProductions.size()));
                    }
                }
            }
            processedNonTerminals.insert(A);
        }

        // Add productions for non-terminals that weren't processed
        for (const auto &prod : productions)
        {
            if (processedNonTerminals.find(prod.left) == processedNonTerminals.end())
            {
                newProductions.push_back(Production(prod.left, prod.right, newProductions.size()));
            }
        }

        productions = newProductions;
    }

    void leftFactor()
    {
        bool changed = true;
        while (changed)
        {
            changed = false;
            map<string, vector<string>> productionMap;

            // Group productions by left-hand side
            for (const auto &prod : productions)
            {
                productionMap[prod.left].push_back(prod.right);
            }

            vector<Production> newProductions;

            for (const auto &entry : productionMap)
            {
                string nonTerm = entry.first;
                vector<string> rights = entry.second;

                map<string, vector<string>> prefixGroups;

                // Group by common prefixes
                for (const string &right : rights)
                {
                    istringstream iss(right);
                    string firstSymbol;
                    iss >> firstSymbol;
                    prefixGroups[firstSymbol].push_back(right);
                }

                for (const auto &group : prefixGroups)
                {
                    if (group.second.size() > 1)
                    {
                        // Found left factoring opportunity
                        string prefix = group.first;
                        string newNonTerm = nonTerm + "'";

                        // Make sure new non-terminal is unique
                        while (nonTerminals.find(newNonTerm) != nonTerminals.end())
                        {
                            newNonTerm += "'";
                        }
                        nonTerminals.insert(newNonTerm);

                        // A -> prefix A'
                        newProductions.push_back(Production(nonTerm, prefix + " " + newNonTerm, newProductions.size()));

                        // A' -> suffix for each suffix
                        for (const string &right : group.second)
                        {
                            string suffix = right.substr(prefix.length());
                            suffix.erase(0, suffix.find_first_not_of(" \t"));
                            if (suffix.empty())
                            {
                                suffix = "epsilon";
                            }
                            newProductions.push_back(Production(newNonTerm, suffix, newProductions.size()));
                        }
                        changed = true;
                    }
                    else
                    {
                        // No left factoring needed
                        newProductions.push_back(Production(nonTerm, group.second[0], newProductions.size()));
                    }
                }
            }

            if (changed)
            {
                productions = newProductions;
            }
        }
    }

    set<string> computeFirstOfString(const vector<string> &symbols)
    {
        set<string> result;

        for (const string &symbol : symbols)
        {
            if (terminals.find(symbol) != terminals.end() || symbol == "epsilon")
            {
                result.insert(symbol);
                break;
            }

            if (nonTerminals.find(symbol) != nonTerminals.end())
            {
                set<string> symbolFirst = firstSets[symbol];
                for (const string &s : symbolFirst)
                {
                    if (s != "epsilon")
                    {
                        result.insert(s);
                    }
                }

                if (symbolFirst.find("epsilon") == symbolFirst.end())
                {
                    break;
                }
            }
        }

        // If all symbols can derive epsilon
        bool allHaveEpsilon = true;
        for (const string &symbol : symbols)
        {
            if (symbol == "epsilon")
                continue;
            if (terminals.find(symbol) != terminals.end())
            {
                allHaveEpsilon = false;
                break;
            }
            if (nonTerminals.find(symbol) != nonTerminals.end() &&
                firstSets[symbol].find("epsilon") == firstSets[symbol].end())
            {
                allHaveEpsilon = false;
                break;
            }
        }

        if (allHaveEpsilon || symbols.empty())
        {
            result.insert("epsilon");
        }

        return result;
    }

    void computeFirstSets()
    {
        // Initialize
        for (const string &term : terminals)
        {
            if (term != "$")
            {
                firstSets[term].insert(term);
            }
        }
        firstSets["epsilon"].insert("epsilon");

        for (const string &nonTerm : nonTerminals)
        {
            firstSets[nonTerm] = set<string>();
        }

        bool changed = true;
        while (changed)
        {
            changed = false;

            for (const auto &prod : productions)
            {
                string A = prod.left;
                // Clean up the right side and parse properly
                string cleanRight = "";
                istringstream tempIss(prod.right);
                string word;
                bool first = true;
                while (tempIss >> word)
                {
                    if (!first)
                        cleanRight += " ";
                    cleanRight += word;
                    first = false;
                }

                istringstream iss(cleanRight);
                string firstSymbol;
                iss >> firstSymbol;

                set<string> oldFirst = firstSets[A];

                if (firstSymbol == "epsilon" || terminals.find(firstSymbol) != terminals.end())
                {
                    firstSets[A].insert(firstSymbol);
                }
                else if (nonTerminals.find(firstSymbol) != nonTerminals.end())
                {
                    for (const string &s : firstSets[firstSymbol])
                    {
                        firstSets[A].insert(s);
                    }
                }

                if (firstSets[A] != oldFirst)
                {
                    changed = true;
                }
            }
        }
    }

    void computeFollowSets()
    {
        // Initialize
        for (const string &nonTerm : nonTerminals)
        {
            followSets[nonTerm] = set<string>();
        }

        // Add $ to start symbol's follow set
        if (!nonTerminals.empty())
        {
            string startSymbol = *nonTerminals.begin();
            followSets[startSymbol].insert("$");
        }

        bool changed = true;
        while (changed)
        {
            changed = false;

            for (const auto &prod : productions)
            {
                string A = prod.left;
                // Clean up the right side and parse properly
                string cleanRight = "";
                istringstream tempIss(prod.right);
                string word;
                bool first = true;
                while (tempIss >> word)
                {
                    if (!first)
                        cleanRight += " ";
                    cleanRight += word;
                    first = false;
                }

                istringstream iss(cleanRight);
                vector<string> symbols;
                string symbol;
                while (iss >> symbol)
                {
                    symbols.push_back(symbol);
                }

                for (int i = 0; i < symbols.size(); i++)
                {
                    string B = symbols[i];

                    if (nonTerminals.find(B) != nonTerminals.end())
                    {
                        set<string> oldFollow = followSets[B];

                        if (i + 1 < symbols.size())
                        {
                            // There's a symbol after B
                            string nextSymbol = symbols[i + 1];
                            if (terminals.find(nextSymbol) != terminals.end())
                            {
                                followSets[B].insert(nextSymbol);
                            }
                            else if (nonTerminals.find(nextSymbol) != nonTerminals.end())
                            {
                                for (const string &s : firstSets[nextSymbol])
                                {
                                    if (s != "epsilon")
                                    {
                                        followSets[B].insert(s);
                                    }
                                }
                                if (firstSets[nextSymbol].find("epsilon") != firstSets[nextSymbol].end())
                                {
                                    for (const string &s : followSets[A])
                                    {
                                        followSets[B].insert(s);
                                    }
                                }
                            }
                        }
                        else
                        {
                            // B is at the end, add FOLLOW(A) to FOLLOW(B)
                            for (const string &s : followSets[A])
                            {
                                followSets[B].insert(s);
                            }
                        }

                        if (followSets[B] != oldFollow)
                        {
                            changed = true;
                        }
                    }
                }
            }
        }
    }

    void constructParseTable()
    {
        // Initialize parse table with empty entries
        parseTable.clear();

        for (const auto &prod : productions)
        {
            string A = prod.left;
            string alpha = prod.right;

            // Get first symbols of alpha
            istringstream iss(alpha);
            vector<string> symbols;
            string symbol;
            while (iss >> symbol)
            {
                symbols.push_back(symbol);
            }

            set<string> firstOfAlpha = computeFirstOfString(symbols);

            // For each terminal a in FIRST(alpha)
            for (const string &a : firstOfAlpha)
            {
                if (a != "epsilon" && terminals.find(a) != terminals.end())
                {
                    parseTable[{A, a}] = prod.index;
                }
            }

            // If epsilon is in FIRST(alpha)
            if (firstOfAlpha.find("epsilon") != firstOfAlpha.end())
            {
                // For each terminal b in FOLLOW(A)
                for (const string &b : followSets[A])
                {
                    if (terminals.find(b) != terminals.end())
                    {
                        parseTable[{A, b}] = prod.index;
                    }
                }
            }
        }
    }

    void printParseTable()
    {
        cout << "\n--- PARSE TABLE ---\n";

        // Print header
        cout << setw(12) << " ";
        for (const string &term : terminals)
        {
            cout << setw(8) << term;
        }
        cout << "\n";

        // Print table rows
        for (const string &nonTerm : nonTerminals)
        {
            cout << setw(12) << nonTerm;
            for (const string &term : terminals)
            {
                auto it = parseTable.find({nonTerm, term});
                if (it != parseTable.end())
                {
                    cout << setw(8) << it->second;
                }
                else
                {
                    cout << setw(8) << "-";
                }
            }
            cout << "\n";
        }
    }

    void createExcelFile()
    {
        ofstream csvFile("parse_table.csv");

        if (!csvFile.is_open())
        {
            cout << "Error: Could not create CSV file.\n";
            return;
        }

        // Write header
        csvFile << "Non-Terminal";
        for (const string &term : terminals)
        {
            csvFile << "," << term;
        }
        csvFile << "\n";

        // Write table rows
        for (const string &nonTerm : nonTerminals)
        {
            csvFile << nonTerm;
            for (const string &term : terminals)
            {
                csvFile << ",";
                auto it = parseTable.find({nonTerm, term});
                if (it != parseTable.end())
                {
                    // Write production rule instead of just index
                    csvFile << "P" << it->second << ": " << productions[it->second].left
                            << " -> " << productions[it->second].right;
                }
                else
                {
                    csvFile << "-";
                }
            }
            csvFile << "\n";
        }

        csvFile.close();

        // Create a detailed productions sheet
        ofstream prodFile("productions.csv");
        if (prodFile.is_open())
        {
            prodFile << "Production Number,Left Side,Right Side\n";
            for (int i = 0; i < productions.size(); i++)
            {
                prodFile << "P" << i << "," << productions[i].left
                         << "," << productions[i].right << "\n";
            }
            prodFile.close();
        }

        // Create FIRST and FOLLOW sets file
        ofstream setsFile("first_follow_sets.csv");
        if (setsFile.is_open())
        {
            setsFile << "Non-Terminal,FIRST Set,FOLLOW Set\n";
            for (const string &nonTerm : nonTerminals)
            {
                setsFile << nonTerm << ",\"";
                bool first = true;
                for (const string &symbol : firstSets[nonTerm])
                {
                    if (!first)
                        setsFile << ", ";
                    setsFile << symbol;
                    first = false;
                }
                setsFile << "\",\"";
                first = true;
                for (const string &symbol : followSets[nonTerm])
                {
                    if (!first)
                        setsFile << ", ";
                    setsFile << symbol;
                    first = false;
                }
                setsFile << "\"\n";
            }
            setsFile.close();
        }

        cout << "\nFiles created successfully:\n";
        cout << "1. parse_table.csv - LL(1) Parse Table\n";
        cout << "2. productions.csv - Grammar Productions\n";
        cout << "3. first_follow_sets.csv - FIRST and FOLLOW Sets\n";
        cout << "\nYou can open these CSV files in Excel or any spreadsheet application.\n";
    }

    void printFirstSets()
    {
        cout << "\n--- FIRST SETS ---\n";
        for (const string &nonTerm : nonTerminals)
        {
            cout << "FIRST(" << nonTerm << ") = { ";
            bool first = true;
            for (const string &symbol : firstSets[nonTerm])
            {
                if (!first)
                    cout << ", ";
                cout << symbol;
                first = false;
            }
            cout << " }\n";
        }
    }

    void printFollowSets()
    {
        cout << "\n--- FOLLOW SETS ---\n";
        for (const string &nonTerm : nonTerminals)
        {
            cout << "FOLLOW(" << nonTerm << ") = { ";
            bool first = true;
            for (const string &symbol : followSets[nonTerm])
            {
                if (!first)
                    cout << ", ";
                cout << symbol;
                first = false;
            }
            cout << " }\n";
        }
    }

public:
    void inputGrammar(int n)
    {
        cout << "Enter " << n << " grammar productions (format: A -> alpha):\n";
        cin.ignore(); // Clear input buffer

        for (int i = 0; i < n; i++)
        {
            string production;
            cout << "Production " << (i + 1) << ": ";
            getline(cin, production);
            parseProduction(production);
        }

        identifySymbols();
    }

    void convertToLL1()
    {
        eliminateLeftRecursion();
        leftFactor();

        cout << "\nLL(1) Grammar:\n";
        printGrammar();

        computeFirstSets();
        printFirstSets();

        computeFollowSets();
        printFollowSets();

        constructParseTable();
        printParseTable();

        createExcelFile();
    }

    void printGrammar()
    {
        for (int i = 0; i < productions.size(); i++)
        {
            cout << "P" << i << ": " << productions[i].left
                 << " -> " << productions[i].right << "\n";
        }
    }
};

int main()
{
    int n;
    cout << "Enter number of grammar productions: ";
    cin >> n;

    GrammarConverter converter;
    converter.inputGrammar(n);
    converter.convertToLL1();

    return 0;
}
