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
    size_t found = 0;
    size_t prev_found = 0;
    while ((found = line.find(delim, found+1)) != string::npos)
    {
        ss << line.substr(prev_found, found - prev_found) << " " << delim << " ";
        prev_found = found + 1;
    }
    ss << line.substr(prev_found,line.size() -  prev_found);
    return ss.str();
}

template<typename T>
void printVect(vector<T> &v)
{
    for (auto &e : v)
    {
        cout << e << endl;
    }
}

//return is success of parsing
bool parseLine(vector<vector<string>> &args, vector<string> &connectors)
{
    string line = "";
    getline(cin,line);

    vector<string> curargs;
    line = formatLine(line, ";");

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

int runCommand(vector<string> &args)
{
    vector <char *> args_cstr(args.size() + 1);
    toCStrVector(args_cstr, args);  

    int status = 0;
    pid_t pid = fork();
    if (pid == 0)
    {
        return execvp(args_cstr[0],&args_cstr[0]);
    }
    else
    {
        waitpid(pid, &status, 0);
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
            bool failure = 0;
            for (size_t i = 0; i < argsv.size(); ++i) {
                auto &args = argsv.at(i);
                if ((i == 0) || (connectors.at(i-1) == ";") ||
                        (connectors.at(i-1) == "&&" && !failure) ||
                        (connectors.at(i-1) == "||" && failure)) 

                {
                    if (args.at(0) == "exit") return 0;
                    runCommand(args);
                }
            }
        }
    }
    return 0;
}
