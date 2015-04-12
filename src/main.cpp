#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>


using namespace std;

int main(int argc, char **argv)
{
    string cmd = "";
    vector<char*> args;
    string input;
    string line = "";
    
    getline(cin,line);
    stringstream ss(line);

    int c = 0;
    while (ss >> input) {

        cout << c << endl;
        if (cmd == "")
            cmd = input;
        else
        {
            args.push_back(&input[0]);
        }
    }
    args.push_back(NULL);
    
    cout << "BEGIN TEST" << endl;

    execvp(cmd.c_str(),&args[0]);
    return 0;
}
