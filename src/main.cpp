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
    line = formatLine(line, "|");
    line = formatLine(line, ">>");
    line = formatLine(line, ">");
    line = formatLine(line, "<<<");
    line = formatLine(line, "<");

    stringstream ss(line);
    string input = "";
    while (ss >> input) {

        if (input == ";" || input == "&&" || input == "||" || input == "|")
        {
            if (curargs.size() == 0)
            {
               cerr << "Syntax error near '" << input << "': command must precede connector/pipe" << endl;
               return false;
            }
            connectors.push_back(input);
            args.push_back(curargs);
            curargs.clear();
        }
        else if (input.find("&") != string::npos)
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

struct IORedir
{
    string osymbol;
    string isymbol;

    string ofile;
    string ifile;
};


void parse_io_redir(vector<string> &args, IORedir &io_struct)
{
    auto itr_1 = find(args.begin(), args.end(), ">");
    auto itr_2 = find(args.begin(), args.end(), ">>");
    auto itr = min(itr_1, itr_2);
    if (itr < args.end() - 1)
    {
        // if there are more > or >> we need to exit
        if (find(itr + 1, args.end(), ">") != args.end() ||
            find(itr + 1, args.end(), ">>") != args.end())
        {
            cerr << "command failed: " << args[0] << ": found multiple output redirection symbols" << endl;
            _exit(1);
        }

        io_struct.ofile = *(itr + 1);
        io_struct.osymbol = *itr;
        
        args.erase(itr + 1);
        args.erase(itr);
    }
    
    itr_1 = find(args.begin(), args.end(), "<");
    itr_2 = find(args.begin(), args.end(), "<<<");
    itr = min(itr_1, itr_2);
    if (itr < args.end() - 1)
    {
        // if there are more < or <<< we need to exit
        if (find(itr + 1, args.end(), "<") != args.end() ||
            find(itr + 1, args.end(), "<<<") != args.end())
        {
            cerr << "command failed: " << args[0] << ": found multiple input redirection symbols" << endl;
            _exit(1);
        }

        //TODO handle strings in quotes for input
        io_struct.ifile = *(itr + 1);
        io_struct.isymbol = *itr;
        
        args.erase(itr + 1);
        args.erase(itr);
    }

}

void redirect(int from, int to)
{
    if (dup2(from, to) == -1)
    {
        cerr << "error redirecting from " << from << " to " << to << endl;
        perror("dup2:" __FILE__ ":" __S_LINE__);
        _exit(1);
    }
    else if (from != to && close(from) == -1)
    {
        cerr << "error closing " << from << endl;
        perror("close:" __FILE__ ":" __S_LINE__);
        _exit(1);
    }
}

//returns pid of command
pid_t run_command(vector<string> &args, int fdi, int fdo, int fdother)
{
    auto pid = fork();
    if (pid == -1)
    {
        perror("fork:" __FILE__ ":" __S_LINE__);
        exit(1);
    }
    else if (pid == 0)
    {
        redirect(fdo, 1);
        redirect(fdi, 0); 
        
        if (fdother != -1 && close(fdother) == -1)
        {
            perror("close:" __FILE__ ":" __S_LINE__);
            exit(1);
        }

        vector <char *> args_cstr(args.size() + 1);
        toCStrVector(args_cstr, args); 
        execvp(args_cstr[0],&args_cstr[0]);
        
        //on failure
        stringstream ss("");
        ss << "command failed: " <<  args_cstr[0];
        perror(ss.str().c_str());
        _exit(errno);
    }
    else
    {
        return pid;
    }
}

int get_output_fd(string file, string symbol)
{
    if (symbol == "") return 1; //stdout

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

    return fd;
 
}

void run_commands(vector<vector<string>> &pipecmds, string const &connector, bool &success)
{
    
    if  (
            (connector == ";") ||
            (connector == "&&" && success) ||
            (connector == "||" && !success)
        )
    {

        vector<pid_t> pids;
        for (auto const &args : pipecmds)
        {
            if (args.at(0) == "exit") exit(0);
        }
        
        int ofd = 1; //stdout
        int ifd = 0; //stdin

        vector<IORedir> redirs(pipecmds.size());
        for (size_t i = 0; i < pipecmds.size(); ++i)
        {
            parse_io_redir(pipecmds.at(i), redirs.at(i));
        }

        // set ofd to file (> or >>) if available
        // do this for last args in pipecmds
        IORedir io_struct_last = redirs.at(redirs.size() - 1);

        for (size_t i = 0; i < pipecmds.size() -1; ++i)
        {   
            int p[2];
            if (pipe(p) == -1)
            {
                perror("pipe:" __FILE__ ":" __S_LINE__);
                exit(1);
            }
            ofd = p[1];

            pids.push_back(run_command(pipecmds.at(i), ifd, ofd, p[0]));
            
            if (ofd != 1 && close(ofd) == -1)
            {
                perror("close:" __FILE__ ":" __S_LINE__);
                exit(1);
            }
            if (ifd != 0 && close(ifd) == -1)
            {
                perror("close:" __FILE__ ":" __S_LINE__);
                exit(1);
            }

            ifd = p[0];
        }
        
        // set ifd to file/string (< or <<<) if available
        // do this for first args in pipecmds
        //TODO
        ofd = get_output_fd(io_struct_last.ofile, io_struct_last.osymbol);
        
        pids.push_back(run_command(pipecmds.at(pipecmds.size() - 1), ifd, ofd, -1));

        if (ofd != 1 && close(ofd) == -1)
        {
            perror("close:" __FILE__ ":" __S_LINE__);
            exit(1);
        }
        if (ifd != 0 && close(ifd) == -1)
        {
            perror("close:" __FILE__ ":" __S_LINE__);
            exit(1);
        }

        success = true;
        for (auto pid : pids)
        { 
            auto error = waitpid(pid, 0, 0);   
            if (error == -1)
            {
                 perror("waitpid:" __FILE__ ":" __S_LINE__);
                 success = false;
            }
        }

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
                vector<vector<string>> pipecmds;
                size_t connector_i = i;
                while (i < connectors.size() && connectors.at(i) == "|")
                {
                    pipecmds.push_back(argsv.at(i));
                    ++i;
                }
                pipecmds.push_back(argsv.at(i));
                
                string connector = connector_i == 0? ";" : connectors.at(connector_i-1);
                run_commands(pipecmds, connector, success);
                 
            }

        }
    }
    return 0;
}
