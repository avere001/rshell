// a vector containing all children of the current process
// must be manually updated everytime a new child is spawned
vector<pid_t> pids;

//
// hanlder for SIGINT
// sends SIGINT to all children in the foreground
//
// @signo the signal to send. must be SIGINT
//
void sigint_handler(int signo)
{
    if (signo != SIGINT) 
    {
        cerr << "unrecognized signal caught by siginthandler!" << endl;
        exit(1);
    }

    for (auto child : pids)
    {
        kill(child, signo);
    }
}
