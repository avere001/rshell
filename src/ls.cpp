#include <unistd.h>
#include<stddef.h>
#include <dirent.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <iomanip>

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
 *  helper functions
 *
 *
 */
void set_flags(char* flags, bool &long_flag, bool &recursive_flag, bool &all_flag)
{
    while ((*flags) != '\0')
    {
        char c = *flags;
        switch (c)
        {
            case 'l':
                long_flag = true;
                break;
            case 'R':
                recursive_flag = true;
                break;
            case 'a':
                all_flag = true;
                break;
            default:
                cout << "error: " << c << " is not a valid flag" << endl;
                cout << "valid flags are: l, R, a" << endl;
                exit(1);
                break;
        }
        flags++;
    }
}

/*
 *
 * major functions
 *
*/
void parse_arguments(int argc, char** argv, 
                        bool &long_flag, bool &recursive_flag, bool &all_flag, 
                        vector<string> &files, vector<string> &directories)
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-' && argv[i][1] != '\0')
        {
            set_flags(&argv[i][1], long_flag, recursive_flag, all_flag);
        }
        else
        {
            if (is_dir(argv[i]))
            {
                directories.push_back(argv[i]);
            }
            else
            {
                files.push_back(argv[i]);
            }
        }
    }
}

//TODO: implement long_flag
void print_files(vector<string> files, bool long_flag)
{
    sort(files.begin(), files.end());

    size_t total_width = 30;
    size_t max_width = 0;
    for (auto &s : files)
    {
        if (s.size() > max_width)
        {
            max_width = s.size();
        }
    }

    size_t files_per_line = total_width/(max_width + 1);
    size_t files_per_col = files.size() / files_per_line;
    
    /*
    //set up a table for printing the files
    vector<vector<string>> filesvect(files_per_line);
    for (auto &v : filesvect)
    {
        v.resize(files_per_col);
    }

    //assign the strings to appropriate rows/cols
    int curr_col = 0;
    for (size_t i = 0; i < files.size(); ++i)
    {
        filesvect.at(i / files_per_col).at(i % files_per_col) = files.at(i);
    }
    */
    files.resize(files_per_line * (files_per_col + 1));

    for (size_t i = 0; i < files_per_col + 1; ++i)
    {
        for (size_t j = 0; j < files_per_line; ++j)
        {
            cout << left << setw(max_width + 1) << files.at(j*(files_per_col+1) + i);
        }
        cout << endl;
    }
    
    
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

    vector<string> files;// = {"a", "b", "ccc", "dddddddd", "eeeee", "f", "g", "h", "i", "j"};
    vector<string> directories;
    
    parse_arguments(argc, argv, 
                    long_flag, recursive_flag, all_flag,
                    files, directories);

    if (files.size() > 0)
    {
        print_files(files, long_flag);
    }
    else if (directories.size() == 1 && !recursive_flag)
    {
        //print contents and quit 
    }
//    else if (directories.size() == 0 && !recursive_flag)
//    {
//        print the current directory
//    }
    
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
