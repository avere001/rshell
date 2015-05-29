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
