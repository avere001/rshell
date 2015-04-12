#include<cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
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

void runCommand(vector<string> &args)
{
    vector <char *> args_cstr(args.size() + 1);
    toCStrVector(args_cstr, args);  


    int status = 0;
    pid_t pid = fork();
    if (pid == 0)
    {
        int error = execvp(args_cstr[0],&args_cstr[0]);
        if (error)
        {
            exit(0); //do nothing
        }
    }
    else
    {
        waitpid(pid, &status, 0);
    }
}

int main(int argc, char **argv)
{
  
    while (true) {
        vector<string> args;
        printPrompt(cout); 
        parseLine(args);       
        if (args[0] == "exit") return 0;
        runCommand(args);
    }
    return 0;
}
