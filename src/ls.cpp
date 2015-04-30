#include<algorithm>
#include <iostream>
#include <vector>

using namespace std;

struct DirNode
{
    string dirname;
    vector<string> files;
    DirNode(string dirname) : dirname(dirname) {}
    bool operator<(DirNode const &other) const
    {
        return dirname < other.dirname;
    }
};

/*
 *
 * major functions
 *
*/
void parse_arguments(int argc, char** argv, 
                        bool &long_flag, bool &recursive_flag, bool &all_flag, 
                        vector<string> &files, vector<string> &directories)
{
    
}

void printFiles(vector<string> const &files, bool long_flag)
{

}

void add_directory(vector<DirNode> &dirvect, string d, bool all_flag)
{

}

void recurse_directory(vector<DirNode> &dirvect, string d, bool all_flag)
{
    

}

void print_directories(vector<DirNode> dirvect, bool long_flag)
{

}
/*
 *
 *  helper functions
 *
 *
 */

//returns whether the file *resembles* a directory
bool is_dir(string file)
{
    //TODO: implement
    return false;
}

int main(int argc, char **argv)
{
    bool long_flag;
    bool recursive_flag;
    bool all_flag;

    vector<string> files;
    vector<string> directories;

    parse_arguments(argc, argv, 
                    long_flag, recursive_flag, all_flag,
                    files, directories);

    if (files.size() > 0)
    {
        //print the files
    }
    else if (directories.size() == 1 && !recursive_flag)
    {
        //print contents and quit 
    }
    else if (directories.size() == 0 && !recursive_flag)
    {
        //print the current directory
    }
    
    vector<DirNode> dirvect;

    for (auto const &d : directories)
    {
        if (recursive_flag)
        {
            recurse_directory(dirvect, d, all_flag);  
        }
        else
        {
            add_directory(dirvect, d, all_flag);
        }
    }

    sort(dirvect.begin(), dirvect.end());

    print_directories(dirvect, long_flag);

    return 0;
}
