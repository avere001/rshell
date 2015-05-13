#include <fcntl.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <sstream>

#define S(x) #x
#define S_(x) S(x)
#define __S_LINE__ S_(__LINE__)

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
    //line = formatLine(line, ">>");
    line = formatLine(line, ">");

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


// process output redirection and modify args accordingly
// only the first >/>> will be processed
// all others will be ignored and remain in args
// if the symbol is the last token, it will also remain
// returns:
//      -1: no redirection occured
//     >=0: the file descriptor
int redirect_out(vector<string> &args)
{
    /*
    size_t i = 0;
    for (i = 0; i < args.size(); ++i)
    {
        if (args.at(i) == ">" || args.at(i) == ">>")
        {
            if find.begin(
            break;

        }
    }
    */
    auto itr_1 = find(args.begin(), args.end(), ">");
    auto itr_2 = find(args.begin(), args.end(), ">>");
    auto &itr = min(itr_1, itr_2);
    if (itr < args.end() - 1)
    {
        // if there are more > or >> we need to exit
        if (find(itr + 1, args.end(), ">") != args.end() ||
            find(itr + 1, args.end(), ">>") != args.end())
        {
            cerr << "command failed: " << args[0] << ": found multiple output redirection symbols" << endl;
            _exit(1);
        }

        string file = *(itr + 1);
        string symbol = *itr;
        
        args.erase(itr + 1);
        args.erase(itr);
        
        int flags = O_CREAT | O_WRONLY;
        mode_t mode = S_IRUSR | S_IWUSR;
        
        if (symbol == ">")
        {
            flags |= O_TRUNC;
        }
        else if (symbol == ">>")
        {
            flags |= O_APPEND;
        }

        int fd = open (file.c_str(), flags, mode);
        if (fd == -1)
        {
            perror("open:" __FILE__ ":" __S_LINE__);
            _exit(errno);
        }

        fd = dup2(fd, 1);
        if (fd  == -1)
        {
            perror("dup2:" __FILE__ ":" __S_LINE__);
            _exit(errno);
        }
        return fd;
    }
    return -1;
}


bool runCommand(vector<string> &args, bool redir_out = true, bool redir_in = true)
{
    int status = 0;
    auto pid = fork();
    if (pid == -1)
    {
        perror("fork:" __FILE__ ":" __S_LINE__);
        return false;
    }
    else if (pid == 0)
    {
        int fd = -1;
        if (redir_out)
        {
            fd = redirect_out(args);
        }
        vector <char *> args_cstr(args.size() + 1);
        toCStrVector(args_cstr, args); 
        execvp(args_cstr[0],&args_cstr[0]);
        //on failure
        stringstream ss("");
        ss << "command failed: " <<  args_cstr[0];
        perror(ss.str().c_str());
        if (fd >= 0)
        {
            if (close(fd) == -1)
            {
                perror("close:" __FILE__ ":" __S_LINE__);
            }
        }
        
        _exit(errno);
    }
    else
    {
        auto error = waitpid(pid, &status, 0);
        if (error == -1)
        {
             perror("waitpid:" __FILE__ ":" __S_LINE__);
             return false;
        }
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
