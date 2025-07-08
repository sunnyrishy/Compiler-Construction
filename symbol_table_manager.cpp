// The below code was developed by Sunny (1987245) and Sai Manoj (1987245)

#include<bits/stdc++.h>
using namespace std;

struct Attributes { //each represent a symbol in the symbol table
    string token_type;
    string data_type; 
    string value;     
    int scope_level;
    int aux_index;  //default -1
};

struct Auxiliary_attributes { //for arrays
    int lower_bound;
    int upper_bound;
    string bound_type;
    string base_type;
    int attribute_index;
};


map<string, Attributes> symbol_table;
vector<Auxiliary_attributes> auxiliary_table;
stack<int> scope_stack;  //to track the nested scope in the code

int current_scope = 0;
//helper functions
// written by both of us
bool is_space(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n';
}

bool is_alphabet(char ch) {
    return isalpha(ch);
}

bool is_digit(char ch) {
    return isdigit(ch);
}

bool is_special(char ch) {
    string specials = ";=(),.[]:";
    return specials.find(ch) != string::npos;
}
//reference : https://www.geeksforgeeks.org/cpp/stringnpos-in-c-with-examples/

bool is_keyword(const string &word) {
    static set<string> keywords = {
        "PROGRAM", "CONST", "VAR", "BEGIN", "END", "ARRAY", "OF", "INTEGER", "REAL"
    };
    return keywords.count(word);
}

string to_upper(const string &s) {
    string res = s;
    for (char &c : res) {
        c = toupper(c);
    }
    return res;
}

string to_lower(const string &s) {
    string res = s;
    for (char &c : res) {
        c = tolower(c);
    }
    return res;
}
//symbol table insertion functions 
//for non array symbol
//written by sunny
void insert_symbol(const string &token, const string &type, const string &value = "", const string &data_type = "", int aux_index = -1) {
    Attributes entry;
    entry.token_type = type;
    entry.value = value;
    entry.data_type = data_type;
    entry.scope_level = current_scope;
    entry.aux_index = aux_index;

    symbol_table[token] = entry;
}

//for array symbol
//written by sunny
void insert_array(const string &name, int lower, int upper, const string &base_type) {
    Attributes entry;
    entry.token_type = "array";
    entry.scope_level = current_scope;
    entry.data_type = base_type;

    Auxiliary_attributes aux;
    aux.lower_bound = lower;
    aux.upper_bound = upper;
    aux.bound_type = "static";
    aux.base_type = base_type;
    aux.attribute_index = auxiliary_table.size();

    auxiliary_table.push_back(aux);
    entry.aux_index = aux.attribute_index;
    symbol_table[name] = entry;
}

//transforms code into tokens and prints token type label
//written by sai manoj
void process_tokens(const vector<string> &tokens) {
    for (int i = 0; i < tokens.size(); ++i) {
        string token = to_upper(tokens[i]);

        if (token == "BEGIN") {
            current_scope++;
            scope_stack.push(current_scope);
            insert_symbol("BEGIN", "keyword");
        } else if (token == "END") {
            insert_symbol("END", "keyword");
            if (!scope_stack.empty()) {
                scope_stack.pop();
                current_scope = scope_stack.empty() ? 0 : scope_stack.top();
            }
        } else if (token == "ARRAY") {
            i++; 
            if (tokens[i] != "[") continue;
            i++;
            int lower = stoi(tokens[i++]);
            i++; 
            int upper = stoi(tokens[i++]);
            i++;
            if (tokens[i++] != "OF") continue;
            string base_type = to_upper(tokens[i++]);
            string var_name = tokens[i++]; 
            insert_array(var_name, lower, upper, base_type);
        } else if (is_keyword(token)) {
            insert_symbol(tokens[i], "keyword");
        } else if (is_digit(token[0])) {
            insert_symbol(tokens[i], "constant", tokens[i]);
        } else if (token.size() == 1 && is_special(token[0])) {
            insert_symbol(tokens[i], "delimiter");
        }
        else if (token == "..") {
            insert_symbol(token, "delimiter"); 
}
        else {
            insert_symbol(tokens[i], "identifier");
        }
    }
}

//this will parse all the tokens and constructs the symbol table
//written by sai manoj
vector<string> tokenize_input(const string &code) {
    vector<string> tokens;
    int i = 0;
    while (i < code.length()) {
        if (is_space(code[i])) {
            i++;
            continue;
        }

        if (is_alphabet(code[i])) {
            string word;
            while (i < code.length() && (is_alphabet(code[i]) || is_digit(code[i])))
                word += code[i++];
            tokens.push_back(word);
            cout << word << " " << (is_keyword(to_upper(word)) ? "tok" + to_lower(word) : "tokidentifier") << endl;
        } else if (is_digit(code[i])) {
            string num;
            while (i < code.length() && is_digit(code[i]))
                num += code[i++];
            tokens.push_back(num);
            cout << num << " toknumber" << endl;
        } else if (code[i] == '.' && i + 1 < code.length() && code[i + 1] == '.') {
            tokens.push_back("..");
            cout << ".. tokdotdot" << endl;
            i += 2;
        } else if (is_special(code[i])) {
            tokens.push_back(string(1, code[i]));
            string desc;
            switch (code[i]) {
                case ';': desc = "toksemicolon"; break;
                case '=': desc = "tokequals"; break;
                case ',': desc = "tokcomma"; break;
                case '(': desc = "toklparen"; break;
                case ')': desc = "tokrparen"; break;
                case '.': desc = "tokperiod"; break;
                case '[': desc = "toklbracket"; break;
                case ']': desc = "tokrbracket"; break;
                case ':': desc = "tokcolon"; break;
                default: desc = "tokop";
            }
            cout << code[i] << " " << desc << endl;
            i++;
        } else {
            i++;
        }
    }
    return tokens;
}

void print_symbol_table() {
    cout << "\nSymbol Table:\n";
    cout << "Name\tType\tScope\tDataType\tValue\tAuxIndex\n";
    for (const auto &[key, val] : symbol_table) {
        cout << key << "\t" << val.token_type << "\t" << val.scope_level << "\t"
             << val.data_type << "\t" << val.value << "\t" << val.aux_index << endl;
    }
}

void print_aux_table() {
    cout << "\nAuxiliary Table:\n";
    cout << "Index\tLower\tUpper\tBaseType\n";
    for (size_t i = 0; i < auxiliary_table.size(); i++) {
        const auto &a = auxiliary_table[i];
        cout << i << "\t" << a.lower_bound << "\t" << a.upper_bound << "\t" << a.base_type << endl;
    }
}


int main () {
    
    string line, program_input;
    cout << "Enter Pascal code (end with empty line):\n";
    while (getline(cin, line)) {
        if (line.empty()) break;
        program_input += line + "\n";
    }

    vector<string> tokens = tokenize_input(program_input);
    process_tokens(tokens);
    print_symbol_table();   //printing symbol table
    print_aux_table();    //printing auxiliary table

    return 0;
}
