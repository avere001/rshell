#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>


using namespace std;

int main(int argc, char **argv)
{
    string cmd = "";
    vector<char*> args;
    string input = "";
    string line = "";
    
    getline(cin,line);
    stringstream ss(line);

    while (ss >> input) {

        if (cmd == "")
            cmd = input;
        else
            args.push_back(&input[0]);
    }
    args.push_back(NULL);

    execvp(cmd.c_str(),&args[0]);
    return 0;
}
