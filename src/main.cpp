#include <algorithm>
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

string formatLine(const string &line, const string &delim)
{
    ostringstream ss("");
    size_t found = line.find(delim);
    size_t prev_found = 0;
    while (found != string::npos)
    {
        ss << line.substr(prev_found, found-prev_found) << " " << delim << " ";
        prev_found = found + delim.size();
        found = line.find(delim, found+delim.size());
    }
    ss << line.substr(prev_found,line.size() -  prev_found);
    return ss.str();
}



bool parseLine(vector<vector<string>> &args, vector<string> &connectors)
{
    string line = "";
    getline(cin,line);

    if (line.find("#") != string::npos)
    {
        line = line.substr(0,line.find("#"));
    }

    vector<string> curargs;
    line = formatLine(line, ";");
    line = formatLine(line, "&&");
    line = formatLine(line, "||");

    stringstream ss(line);
    string input = "";
    while (ss >> input) {
        if (input == ";" || input == "&&" || input == "||")
        {
            if (curargs.size() == 0)
            {
               cerr << "Syntax error near '" << input << "': command must precede connector" << endl;
               return false;
            }
            connectors.push_back(input);
            args.push_back(curargs);
            curargs.clear();
        }
        else if (input.find("&") != string::npos || input.find("|") != string::npos)
        {
            size_t i = min(input.find("&"), input.find("|"));
            cerr << "Syntax error near '" << input[i] << "': invalid connector" << endl;
            return false;
        }
        else
        {
            curargs.push_back(input);
        }
    }
    if (curargs.size() > 0)
    {
        args.push_back(curargs);
    }
    return true;
}

bool runCommand(vector<string> &args)
{
    vector <char *> args_cstr(args.size() + 1);
    toCStrVector(args_cstr, args);  

    int status = 0;
    auto pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return false;
    }
    else if (pid == 0)
    {
        execvp(args_cstr[0],&args_cstr[0]);
        //on failure
        stringstream ss("");
        ss << "command failed: " <<  args_cstr[0];
        perror(ss.str().c_str());
        _exit(errno);
    }
    else
    {
        waitpid(pid, &status, 0);
        return status == 0;
    }
}

int main(int argc, char **argv)
{
    while (true) {
        vector<vector<string>> argsv;
        vector<string> connectors;
        printPrompt(cout); 
        if (parseLine(argsv,connectors))
        {
            bool success = true;
            for (size_t i = 0; i < argsv.size(); ++i) {
                auto &args = argsv.at(i);
                if ((i == 0) || (connectors.at(i-1) == ";") ||
                        (connectors.at(i-1) == "&&" && success) ||
                        (connectors.at(i-1) == "||" && !success)) 

                {
                    if (args.at(0) == "exit") return 0;
                    success = runCommand(args);
                }
            }
        }
    }
    return 0;
}
