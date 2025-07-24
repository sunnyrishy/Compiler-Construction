#include<bits/stdc++.h>
using namespace std;

struct Production {
    string left;
    string right;
    
    Production(string l, string r) : left(l), right(r) {}
};

class GrammarConverter {
private:
    vector<Production> productions;
    set<string> nonTerminals;
    set<string> terminals;
    map<string, set<string>> firstSets;
    map<string, set<string>> followSets;
    
    void parseProduction(string input) {
        size_t arrowPos = input.find("->");
        if (arrowPos == string::npos) return;
        
        string left = input.substr(0, arrowPos);
        string right = input.substr(arrowPos + 2);
        
        // Trim whitespace
        left.erase(0, left.find_first_not_of(" \t"));
        left.erase(left.find_last_not_of(" \t") + 1);
        right.erase(0, right.find_first_not_of(" \t"));
        right.erase(right.find_last_not_of(" \t") + 1);
        
        // Handle special case for parentheses - split (E) into ( E )
        string processedRight = "";
        for (int i = 0; i < right.length(); i++) {
            if (right[i] == '(' || right[i] == ')') {
                processedRight += " ";
                processedRight += right[i];
                processedRight += " ";
            } else {
                processedRight += right[i];
            }
        }
        
        productions.push_back(Production(left, processedRight));
        nonTerminals.insert(left);
    }
    
    void identifySymbols() {
        for (const auto& prod : productions) {
            istringstream iss(prod.right);
            string symbol;
            while (iss >> symbol) {
                if (nonTerminals.find(symbol) == nonTerminals.end()) {
                    terminals.insert(symbol);
                }
            }
        }
    }
    
    bool hasLeftRecursion(const string& nonTerm) {
        for (const auto& prod : productions) {
            if (prod.left == nonTerm) {
                istringstream iss(prod.right);
                string firstSymbol;
                iss >> firstSymbol;
                if (firstSymbol == nonTerm) {
                    return true;
                }
            }
        }
        return false;
    }
    
    void eliminateLeftRecursion() {
        vector<Production> newProductions;
        set<string> processedNonTerminals;
        
        for (const string& A : nonTerminals) {
            if (processedNonTerminals.find(A) != processedNonTerminals.end()) {
                continue;
            }
            
            vector<string> alpha, beta;
            
            // Collect alpha (left recursive) and beta (non-left recursive) productions
            for (const auto& prod : productions) {
                if (prod.left == A) {
                    string processedRight = prod.right;
                    // Clean up extra spaces
                    istringstream iss(processedRight);
                    string cleanRight = "";
                    string word;
                    bool first = true;
                    while (iss >> word) {
                        if (!first) cleanRight += " ";
                        cleanRight += word;
                        first = false;
                    }
                    
                    istringstream iss2(cleanRight);
                    string firstSymbol;
                    iss2 >> firstSymbol;
                    
                    if (firstSymbol == A) {
                        // Left recursive: A -> A alpha
                        string rest = prod.right.substr(firstSymbol.length());
                        rest.erase(0, rest.find_first_not_of(" \t"));
                        alpha.push_back(rest);
                    } else {
                        // Non-left recursive: A -> beta
                        beta.push_back(prod.right);
                    }
                }
            }
            
            if (!alpha.empty()) {
                // Has left recursion, need to eliminate
                string newNonTerm = A + "'";
                nonTerminals.insert(newNonTerm);
                
                // A -> beta A'
                for (const string& b : beta) {
                    newProductions.push_back(Production(A, b + " " + newNonTerm));
                }
                
                // A' -> alpha A' | epsilon
                for (const string& a : alpha) {
                    newProductions.push_back(Production(newNonTerm, a + " " + newNonTerm));
                }
                newProductions.push_back(Production(newNonTerm, "epsilon"));
            } else {
                // No left recursion, keep original productions
                for (const auto& prod : productions) {
                    if (prod.left == A) {
                        newProductions.push_back(prod);
                    }
                }
            }
            processedNonTerminals.insert(A);
        }
        
        // Add productions for non-terminals that weren't processed
        for (const auto& prod : productions) {
            if (processedNonTerminals.find(prod.left) == processedNonTerminals.end()) {
                newProductions.push_back(prod);
            }
        }
        
        productions = newProductions;
    }
    
    void leftFactor() {
        bool changed = true;
        while (changed) {
            changed = false;
            map<string, vector<string>> productionMap;
            
            // Group productions by left-hand side
            for (const auto& prod : productions) {
                productionMap[prod.left].push_back(prod.right);
            }
            
            vector<Production> newProductions;
            
            for (const auto& entry : productionMap) {
                string nonTerm = entry.first;
                vector<string> rights = entry.second;
                
                map<string, vector<string>> prefixGroups;
                
                // Group by common prefixes
                for (const string& right : rights) {
                    istringstream iss(right);
                    string firstSymbol;
                    iss >> firstSymbol;
                    prefixGroups[firstSymbol].push_back(right);
                }
                
                for (const auto& group : prefixGroups) {
                    if (group.second.size() > 1) {
                        // Found left factoring opportunity
                        string prefix = group.first;
                        string newNonTerm = nonTerm + "'";
                        
                        // Make sure new non-terminal is unique
                        while (nonTerminals.find(newNonTerm) != nonTerminals.end()) {
                            newNonTerm += "'";
                        }
                        nonTerminals.insert(newNonTerm);
                        
                        // A -> prefix A'
                        newProductions.push_back(Production(nonTerm, prefix + " " + newNonTerm));
                        
                        // A' -> suffix for each suffix
                        for (const string& right : group.second) {
                            string suffix = right.substr(prefix.length());
                            suffix.erase(0, suffix.find_first_not_of(" \t"));
                            if (suffix.empty()) {
                                suffix = "epsilon";
                            }
                            newProductions.push_back(Production(newNonTerm, suffix));
                        }
                        changed = true;
                    } else {
                        // No left factoring needed
                        newProductions.push_back(Production(nonTerm, group.second[0]));
                    }
                }
            }
            
            if (changed) {
                productions = newProductions;
            }
        }
    }
    
    set<string> computeFirstOfString(const vector<string>& symbols) {
        set<string> result;
        
        for (const string& symbol : symbols) {
            if (terminals.find(symbol) != terminals.end() || symbol == "epsilon") {
                result.insert(symbol);
                break;
            }
            
            if (nonTerminals.find(symbol) != nonTerminals.end()) {
                set<string> symbolFirst = firstSets[symbol];
                for (const string& s : symbolFirst) {
                    if (s != "epsilon") {
                        result.insert(s);
                    }
                }
                
                if (symbolFirst.find("epsilon") == symbolFirst.end()) {
                    break;
                }
            }
        }
        
        // If all symbols can derive epsilon
        bool allHaveEpsilon = true;
        for (const string& symbol : symbols) {
            if (symbol == "epsilon") continue;
            if (terminals.find(symbol) != terminals.end()) {
                allHaveEpsilon = false;
                break;
            }
            if (nonTerminals.find(symbol) != nonTerminals.end() && 
                firstSets[symbol].find("epsilon") == firstSets[symbol].end()) {
                allHaveEpsilon = false;
                break;
            }
        }
        
        if (allHaveEpsilon || symbols.empty()) {
            result.insert("epsilon");
        }
        
        return result;
    }
    
    void computeFirstSets() {
        // Initialize
        for (const string& term : terminals) {
            firstSets[term].insert(term);
        }
        firstSets["epsilon"].insert("epsilon");
        
        for (const string& nonTerm : nonTerminals) {
            firstSets[nonTerm] = set<string>();
        }
        
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (const auto& prod : productions) {
                string A = prod.left;
                // Clean up the right side and parse properly
                string cleanRight = "";
                istringstream tempIss(prod.right);
                string word;
                bool first = true;
                while (tempIss >> word) {
                    if (!first) cleanRight += " ";
                    cleanRight += word;
                    first = false;
                }
                
                istringstream iss(cleanRight);
                string firstSymbol;
                iss >> firstSymbol;
                
                set<string> oldFirst = firstSets[A];
                
                if (firstSymbol == "epsilon" || terminals.find(firstSymbol) != terminals.end()) {
                    firstSets[A].insert(firstSymbol);
                } else if (nonTerminals.find(firstSymbol) != nonTerminals.end()) {
                    for (const string& s : firstSets[firstSymbol]) {
                        firstSets[A].insert(s);
                    }
                }
                
                if (firstSets[A] != oldFirst) {
                    changed = true;
                }
            }
        }
    }
    
    void computeFollowSets() {
        // Initialize
        for (const string& nonTerm : nonTerminals) {
            followSets[nonTerm] = set<string>();
        }
        
        // Add $ to start symbol's follow set
        if (!nonTerminals.empty()) {
            string startSymbol = *nonTerminals.begin();
            followSets[startSymbol].insert("$");
        }
        
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (const auto& prod : productions) {
                string A = prod.left;
                // Clean up the right side and parse properly
                string cleanRight = "";
                istringstream tempIss(prod.right);
                string word;
                bool first = true;
                while (tempIss >> word) {
                    if (!first) cleanRight += " ";
                    cleanRight += word;
                    first = false;
                }
                
                istringstream iss(cleanRight);
                vector<string> symbols;
                string symbol;
                while (iss >> symbol) {
                    symbols.push_back(symbol);
                }
                
                for (int i = 0; i < symbols.size(); i++) {
                    string B = symbols[i];
                    
                    if (nonTerminals.find(B) != nonTerminals.end()) {
                        set<string> oldFollow = followSets[B];
                        
                        if (i + 1 < symbols.size()) {
                            // There's a symbol after B
                            string nextSymbol = symbols[i + 1];
                            if (terminals.find(nextSymbol) != terminals.end()) {
                                followSets[B].insert(nextSymbol);
                            } else if (nonTerminals.find(nextSymbol) != nonTerminals.end()) {
                                for (const string& s : firstSets[nextSymbol]) {
                                    if (s != "epsilon") {
                                        followSets[B].insert(s);
                                    }
                                }
                                if (firstSets[nextSymbol].find("epsilon") != firstSets[nextSymbol].end()) {
                                    for (const string& s : followSets[A]) {
                                        followSets[B].insert(s);
                                    }
                                }
                            }
                        } else {
                            // B is at the end, add FOLLOW(A) to FOLLOW(B)
                            for (const string& s : followSets[A]) {
                                followSets[B].insert(s);
                            }
                        }
                        
                        if (followSets[B] != oldFollow) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    
    void printFirstSets() {
        cout << "\n--- FIRST SETS ---\n";
        for (const string& nonTerm : nonTerminals) {
            cout << "FIRST(" << nonTerm << ") = { ";
            bool first = true;
            for (const string& symbol : firstSets[nonTerm]) {
                if (!first) cout << ", ";
                cout << symbol;
                first = false;
            }
            cout << " }\n";
        }
    }
    
    void printFollowSets() {
        cout << "\n--- FOLLOW SETS ---\n";
        for (const string& nonTerm : nonTerminals) {
            cout << "FOLLOW(" << nonTerm << ") = { ";
            bool first = true;
            for (const string& symbol : followSets[nonTerm]) {
                if (!first) cout << ", ";
                cout << symbol;
                first = false;
            }
            cout << " }\n";
        }
    }
    
public:
    void inputGrammar(int n) {
        cout << "Enter " << n << " grammar productions (format: A -> alpha):\n";
        cin.ignore(); // Clear input buffer
        
        for (int i = 0; i < n; i++) {
            string production;
            cout << "Production " << (i + 1) << ": ";
            getline(cin, production);
            parseProduction(production);
        }
        
        identifySymbols();
    }
    
    void convertToLL1() {
        eliminateLeftRecursion();
        leftFactor();
        
        cout << "\nLL(1) Grammar:\n";
        printGrammar();
        
        computeFirstSets();
        printFirstSets();
        
        computeFollowSets();
        printFollowSets();
    }
    
    void printGrammar() {
        for (const auto& prod : productions) {
            cout << prod.left << " -> " << prod.right << "\n";
        }
    }
};

int main() {
    int n;
    cout << "Enter number of grammar productions: ";
    cin >> n;
    
    GrammarConverter converter;
    converter.inputGrammar(n);
    converter.convertToLL1();
    
    return 0;
}
