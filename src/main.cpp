#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>

using namespace std;

void toCStrVector(vector<char *> &buffer, vector<string> &original)
{
    for (size_t i = 0; i < original.size(); ++i)
    {
        buffer[i] = &original[i][0];
    }
    buffer.push_back(NULL);
}
    
void printPrompt(ostream &os)
{
    os << "$ ";
}

void parseLine(vector<string> &args)
{
    string input = "";
    string line = "";

    getline(cin,line);
    stringstream ss(line);

    while (ss >> input) {
        args.push_back(input);
    }
}

int main(int argc, char **argv)
{
    vector<string> args;
  
    printPrompt(cout); 
    parseLine(args);
    vector <char *> args_cstr(args.size() + 1);
    toCStrVector(args_cstr, args);  
    
    execvp(args_cstr[0],&args_cstr[0]);
    return 0;
}
