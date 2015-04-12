#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>


using namespace std;

int main(int argc, char **argv)
{
    vector<string> args;
    string input = "";
    string line = "";
    
    getline(cin,line);
    stringstream ss(line);

    while (ss >> input) {
        args.push_back(input);
    }

    vector <char *> args_cstr(args.size() + 1);
    for (size_t i = 0; i < args.size(); ++i)
    {
        args_cstr[i] = &args[i][0];
        cout << args[i] << endl;
        cout << args_cstr[i] << endl;
    }
    args_cstr.push_back(NULL);

    execvp(args_cstr[0],&args_cstr[0]);
    return 0;
}
