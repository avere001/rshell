#include <fcntl.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <signal.h>

using namespace std;
#define S(x) #x
#define S_(x) S(x)
#define __S_LINE__ S_(__LINE__)

//TODO fix these to work without using namespace std
#include "IORedir.h"
#include "parsing.h"
#include "redirection.h"
#include "handlers.h"

//
// converts a vector of strings to a vector of c style strings
//
//  NOTE: This is a shallow copy, the output c strings will
//  point to the same place in memory as the original strings
//
//  WARNING: never modify the output vector
//
//  WARNING: Do not modify the original vector before you are
//  finished with the c string vector! 
//  
//
//  @buffer     an empty c string vector to fill with the converted strings
//  @original   a vector of strings to be converted to c strings
//
void toCStrVector(vector<char *> &buffer, vector<string> &original)
{
    for (size_t i = 0; i < original.size(); ++i)
    {
        buffer[i] = &original[i][0];
    }
    buffer.push_back(NULL);
}

//
// prints the prompt
// this is displayed before every line that the
// shell is waiting for user input
// should be "PWD$" where PWD is the current working directory
//
//
bool printPrompt()
{

    /*char *user_cstr = getlogin();
    char *host_cstr = gethostname();

    string user, host;
    if (user_cstr != 0) user = user_cstr;
    if (host_cstr != 0) host = host_cstr;
    */

    cout << getenv("PWD") <<  "$ ";
    return true;
}

//
// changes directory
//
// when passed with no arguments, will cd to HOME
// when passed with -, will cd to the last directory that was cded to
// when passed with anything else, cd will attempt to change to that directory
//
// @args the arguments to cd. args[0] is always "cd"
// 
// returns:
//      -1: successful
//      -2: failure
//
int run_cd(vector<string> &args)
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
        char *cwd = getcwd(nullptr,0);
        if (cwd == nullptr)
        {
            perror("cwd:" __FILE__ ":" __S_LINE__);
            return -2;
        }
        
        setenv("OLDPWD", getenv("PWD"), true);
        setenv("PWD", cwd, true);
        if (printpath) cout << path << endl;
        free (cwd);
    }
    return -1;

}

//
//  executes a command in a child process
//
//  @args       command to run and its argument
//  @fdi        file descriptor to redirect stdin (use 0 for no redirection)
//  @fdo        file descriptor to redirect stdout (use 1 for no redirection)
//  @fde        file descriptor to redirect stderr (use 2 for no redirection
//  @fdother    file descriptor to redirect to be closed by child (use -1 for no closing)
//
//
//  returns:
//      pid of command or
//      -1 for successful cd
//      -2 for failed cd
pid_t run_command(vector<string> &args, int fdi, int fdo, int fde,int fdother)
{
    if (args.at(0) == "cd")
    {
        return run_cd(args);    
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
            _exit(1);
        }
        else
        {
            return pid;
        }
    }
}



void run_commands(vector<vector<string>> &pipecmds, string const &connector, bool &success)
{
    
    if  (
            (connector == ";") ||
            (connector == "&&" && success) ||
            (connector == "||" && !success)
        )
    {

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
                pid_t error;
                
                //wait until program has closed ignoring EINTR
                while ((error = waitpid(pid, &status, 0)) == -1 && errno == EINTR);
                                
                if (error == -1 )
                {
                    perror("waitpid:" __FILE__ ":" __S_LINE__);
                    success = false;
                }
                else if (WIFSIGNALED(status))
                {
                    success = false;
                }
                else if (WIFEXITED(status))
                {
                    if (WEXITSTATUS(status) != 0)
                    {
                        success = false;
                    }
                }
                else
                {
                    cerr << "child program exited abnormally: pid = "  << pid << endl;
                    success = false;
                }
            }
        }
        pids.clear();

    }
}

int main(int argc, char **argv)
{
    struct sigaction sigact;
    sigact.sa_handler = sigint_handler;
    sigaction(SIGINT, &sigact, nullptr);

    bool has_printed = false;
    while (true)
    {

        string line = "";
        while ((has_printed || printPrompt()) && getline(cin,line)) {
            
            
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
