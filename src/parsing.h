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

