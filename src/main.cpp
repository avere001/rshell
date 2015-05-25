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
    
bool printPrompt(ostream &os)
{
    os << "$ ";
    return true;
}

string replaceAll(const string &line, const string &delim, string replacement = "")
{
    if (replacement == "")
    {
        replacement = delim;
    }
    ostringstream ss("");
    size_t found = line.find(delim);
    size_t prev_found = 0;
    while (found != string::npos)
    {
        ss << line.substr(prev_found, found-prev_found) << replacement;
        prev_found = found + delim.size();
        found = line.find(delim, found+delim.size());
    }
    ss << line.substr(prev_found,line.size() -  prev_found);
    return ss.str();
}



bool parseLine(vector<vector<string>> &args, vector<string> &connectors, string &line)
{
    if (line.find("#") != string::npos)
    {
        line = line.substr(0,line.find("#"));
    }

    vector<string> curargs;
    line = replaceAll(line, ";", " ; ");
    line = replaceAll(line, "&&", " && ");
    line = replaceAll(line, "||", "__PLACEHOLDER_OR__");
    line = replaceAll(line, "|", " | ");
    line = replaceAll(line, "1>>", "__PLACEHOLDER_OREDIR1__");
    line = replaceAll(line, "2>>", "__PLACEHOLDER_OREDIR2__");
    line = replaceAll(line, ">>", "__PLACEHOLDER_OREDIR__");
    line = replaceAll(line, "1>", "__PLACEHOLDER_OREDIRT1__");
    line = replaceAll(line, "2>", "__PLACEHOLDER_OREDIRT2__");
    line = replaceAll(line, ">", " > ");
    line = replaceAll(line, "<<<", "__PLACEHOLDER_IREDIR__");
    line = replaceAll(line, "<", " < ");

    line = replaceAll(line, "__PLACEHOLDER_OR__", " || ");
    line = replaceAll(line, "__PLACEHOLDER_OREDIR__", " >> ");
    line = replaceAll(line, "__PLACEHOLDER_OREDIR1__", " 1>> ");
    line = replaceAll(line, "__PLACEHOLDER_OREDIR2__", " 2>> ");
    line = replaceAll(line, "__PLACEHOLDER_IREDIR__", " <<< ");
    line = replaceAll(line, "__PLACEHOLDER_OREDIRT1__", " 1> ");
    line = replaceAll(line, "__PLACEHOLDER_OREDIRT2__", " 2> ");

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
            size_t i = input.find("&");
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
    string ofile;
    
    string esymbol;
    string efile;

    string isymbol;
    string ifile;
};

void find_first(vector<string> &args, vector<string> const &symbols, vector<string>::iterator &itr)
{
    for (auto &s : symbols)
    {
        itr = min(find(args.begin(), args.end(), s), itr);
    }
}

bool parse_io_redir(vector<string> &args, IORedir &io_struct)
{
    while (true)    
    {
        vector<string> const static symbols = {">", ">>", "1>", "1>>"};
        auto itr = args.end();
        find_first(args, symbols, itr);

        if (itr == args.end())
            break;
        else if (itr == args.end() - 1)
        {
            cerr << "unexpected output redirection symbol" << endl;
            return false;
        }

        io_struct.ofile = *(itr + 1);
        io_struct.osymbol = *itr;
        
        args.erase(itr + 1);
        args.erase(itr);
    }
   
    while (true)    
    {
        vector<string> const static symbols = {"2>", "2>>"};
        auto itr = args.end();
        find_first(args, symbols, itr);

        if (itr == args.end())
            break;
        else if (itr == args.end() - 1)
        {
            cerr << "unexpected output redirection symbol" << endl;
            return false;
        }

        io_struct.efile = *(itr + 1);
        io_struct.esymbol = *itr;
        
        args.erase(itr + 1);
        args.erase(itr);
    }
    
    auto itr_1 = find(args.begin(), args.end(), "<");
    auto itr_2 = find(args.begin(), args.end(), "<<<");
    auto itr = min(itr_1, itr_2);
    if (itr < args.end())
    {
        if (itr == args.end() - 1)
        {
            cerr << "unexpected input redirection symbol" << endl;
            return false;
        }

        // if there are more < or <<< we need to exit
        if (find(itr + 1, args.end(), "<") != args.end() ||
            find(itr + 1, args.end(), "<<<") != args.end())
        {
            cerr << "command failed: " << args[0] << ": found multiple input redirection symbols" << endl;
            return false;
        }

        //TODO handle strings in quotes for input
        io_struct.ifile = *(itr + 1);
        io_struct.isymbol = *itr;
        
        args.erase(itr + 1);
        args.erase(itr);
    }

    return true;

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
pid_t run_command(vector<string> &args, int fdi, int fdo, int fde,int fdother)
{
    if (args.at(0) == "cd")
    {
        if (args.size() > 2)
        {
            cerr << "cd: Too many arguments!" << endl;
            cerr << "usage: cd" << endl;
            cerr << "usage: cd <path>" << endl;
            cerr << "usage: cd -" << endl;
            return -2;
        }

        string path;
        bool printpath = false;

        if (args.size() == 1)
        {    
            path = getenv("HOME");
        }
        else if  (args.at(1) == "-")
        {
            path = getenv("OLDPWD");
            printpath = true;
        }
        else
        {
            path = args.at(1);
        }

        if (chdir(path.c_str()) == -1)
        {
            perror("cd:" __FILE__ ":" __S_LINE__);
            return -2;
        }
        else
        {
            setenv("OLDPWD", getenv("PWD"), true);
            setenv("PWD", path.c_str(), true);
            if (printpath) cout << path << endl;
        }
        return -1;

    }
    else
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
            redirect(fde, 2);
            
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
}

int get_input_fd(string file, string symbol)
{
    if (symbol == "") return 0; //stdin

    if (symbol == "<")
    {
        int fd = open (file.c_str(), O_RDONLY);
        if (fd == -1)
        {
            perror("open:" __FILE__ ":" __S_LINE__);
            return 0; //stdin
        }
        return fd;
    }
    else if (symbol == "<<<")
    {
        int p[2];
        if (pipe(p) == -1)
        {
            perror("pipe:" __FILE__ ":" __S_LINE__);
            exit(1);
        }

        if (write(p[1], file.c_str(), file.size()) == -1)
        {
            perror("write:" __FILE__ ":" __S_LINE__);
            exit(1);
        }

        if (close(p[1]) == -1)
        {
            perror("close:" __FILE__ ":" __S_LINE__);
            exit(1);
        }

        return p[0];

    }

    

    return 0; 
}

int get_output_fd(string const &symbol, string const &file, int default_fd)
{
    if (symbol == "") return default_fd;

    else
    {
        int flags = O_CREAT | O_WRONLY;
        mode_t mode = S_IRUSR | S_IWUSR;
        
        string new_symbol = symbol;
        if (symbol.at(0) == '1' || symbol.at(0) == '2')
        {
            new_symbol = symbol.substr(1);
        }

        if (new_symbol == ">")
        {
            flags |= O_TRUNC;
        }
        else if (new_symbol == ">>")
        {
            flags |= O_APPEND;
        }

        int fd = open (file.c_str(), flags, mode);
        if (fd == -1)
        {
            perror("open:" __FILE__ ":" __S_LINE__);
            return default_fd;
        }

        return fd;
    }
}

void get_output_fd(IORedir &s, int &ofd, int &efd)
{
    ofd = get_output_fd(s.osymbol, s.ofile, 1);
    efd = get_output_fd(s.esymbol, s.efile, 2);
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
            if (!parse_io_redir(pipecmds.at(i), redirs.at(i)))
            {
                success = false;
                return;
            }
        }

        //check for inapropriate stdin
        for (size_t i = 1; i < redirs.size(); ++i)
        {
            if (redirs.at(i).ifile != "")
            {
                cerr << "error: stdin input redirection on command other than first" << endl;
                success= false;
                return;
            }
        }

        //inapropriate stdout
        for (size_t i = 0; i < redirs.size() - 1; ++i)
        {
            if (redirs.at(i).ofile != "")
            {
                cerr << "error: stdout output redirection on command other than last" << endl;
                success = false;
                return;
            }
        }

        IORedir io_struct_first = redirs.at(0);
        IORedir io_struct_last = redirs.at(redirs.size() - 1);
        
        // set ifd to file (< or <<) if available
        // do this only for first args in pipecmds
        ifd = get_input_fd(io_struct_first.ifile, io_struct_first.isymbol);

        for (size_t i = 0; i < pipecmds.size() -1; ++i)
        {   
            int p[2];
            if (pipe(p) == -1)
            {
                perror("pipe:" __FILE__ ":" __S_LINE__);
                exit(1);
            }
            ofd = p[1];

            int efd = get_output_fd(redirs.at(i).esymbol, redirs.at(i).efile, 2);
            pids.push_back(run_command(pipecmds.at(i), ifd, ofd, efd, p[0]));
            
            if (efd != 2 && close(efd) == -1)
            {
                perror("close:" __FILE__ ":" __S_LINE__);
                exit(1);
            }
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

        int efd = 2;
        get_output_fd(io_struct_last, ofd, efd);
        
        pids.push_back(run_command(pipecmds.at(pipecmds.size() - 1), ifd, ofd, efd, -1));

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
        if (efd != 2 && close(efd) == -1)
        {
            perror("close:" __FILE__ ":" __S_LINE__);
            exit(1);
        }
        
        success = true;
        for (auto pid : pids)
        {
            if (pid == -1); //cd succeeded
            else if (pid == -2) //cd failed
            {
                success = false;
            }
            else
            {
                int status; 
                auto error = waitpid(pid, &status, 0);   
                if (error == -1 )
                {
                     perror("waitpid:" __FILE__ ":" __S_LINE__);
                     success = false;
                }
                if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
                {
                    success = false;
                }
            }
        }

    }
}

int main(int argc, char **argv)
{
    bool has_printed = false;
    while (true)
    {

        string line = "";
        while ((has_printed || printPrompt(cout)) && getline(cin,line)) {
            
            
            vector<vector<string>> argsv;
            vector<string> connectors;
            
            if (parseLine(argsv,connectors,line))
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
                    //string connector = ";";
                    run_commands(pipecmds, connector, success);
                     
                }

            }

            has_printed = false;
        }
        has_printed = true;
        cin.clear();
    
    }
    return 0;
}
