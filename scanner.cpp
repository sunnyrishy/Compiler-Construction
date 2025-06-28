#include <bits/stdc++.h>
using namespace std;

bool is_space(char ch) // this function will check if the input character is space or not
{
    if (ch == ' ')
        return true;
    if (ch == '\t')
        return true; // horizontal tab space
    if (ch == '\n')
        return true; // newline
    return false;
}

bool is_alphabet(char ch) // this function will check if the input character is a letter or not
{
    if (ch >= 'A' && ch <= 'Z') // A to Z including A and Z
        return true;
    if (ch >= 'a' && ch <= 'z') // a to z including a and z
        return true;
    return false;
}

bool is_digit(char ch) // this function will check if the input character is a digit or not
{
    if (ch >= '0' && ch <= '9') // 0 to 9 including 0 and 9
        return true;
    return false;
}

void gettoken(const string &program_input) // this function takes the input and separates each as tokens
{
    int i = 0;
    while (i < program_input.length()) // i should not cross the input length
    {
        if ((is_space(program_input[i]))) // checking if input character is space or not
        {
            i++;
            continue;
        }

        if (is_alphabet(program_input[i])) // check if input character is letter or not
        {
            string word; // if letter, then consider it as a stream of letters and make it as a word
            while (i < program_input.length() && is_alphabet(program_input[i]))
            {
                word += program_input[i];
                i++;
            }
            cout << word << " " << "tokword" << endl;
        }

        else if (is_digit(program_input[i])) // check if input character is digit or not
        {
            string number; // concat multiple digits to form a number
            while (i < program_input.length() && is_digit(program_input[i]))
            {
                number += program_input[i];
                i++;
            }
            cout << number << " " << "toknumber" << endl;
        }

        else // anything that is not a letter, space, digit is taken as operator token
        {
            cout << program_input[i] << " " << "tokop" << endl;
            i++;
        }
    }
}

int main()
{
    string line;
    string program_input;
    cout << "Enter your code (end your code with an empty line) : " << endl;

    while (getline(cin, line)) // gets each line of code into the "line" string
    {
        if (line.empty()) // if the code reaches end   //something like a end of input token
            break;
        program_input += line + "\n";
    }

    gettoken(program_input); // passing the input to the function
    return 0;
}





Sample Input : 

PROGRAM Sample (Input, Output);
  CONST x  = 15;
  BEGIN
  END;

Output :   -----------------

  PROGRAM tokword
  Sample tokword
  ( tokop
  Input tokword
  , tokop
  Output tokword
  ) tokop
  ; tokop
  CONST tokword
  x tokword
  = tokop
  15 toknumber
  ; tokop
  BEGIN tokword
  END tokword
  ; tokop

