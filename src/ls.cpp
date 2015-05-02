#include <unistd.h>
#include<stddef.h>
#include <dirent.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>

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

//returns whether the file *resembles* a directory
bool is_dir(string file)
{
    struct stat s;
    stat(file.c_str(), &s);
    return S_ISDIR(s.st_mode);
}

//returns whether the file is accessible
bool can_access(string file)
{
    struct stat s;
    if (stat(file.c_str(), &s) == -1)
    {
        perror("stat");
        return false;
    }
    return true;
}

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

void list_dir(string const &d, vector<string> &files, bool all_flag)
{
    DIR* d_ptr = opendir(d.c_str());
    if (d_ptr == NULL)
    {
        cerr << "error opening " << d << endl;
        perror("opendir");
        return;
    }

    struct dirent *prev = new dirent;
    struct dirent *curr = nullptr;

    while (true)
    {
        if (readdir_r(d_ptr, prev, &curr) != 0)
        {
            perror("readdir_r");
        }

        if (curr == nullptr)
        {
            break;
        }
        
        string curr_name = curr->d_name;
        if (curr_name.at(0) != '.' || all_flag)
        {
            files.push_back(curr_name);
        }
    }

    delete prev;
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
        else if (can_access(argv[i])) 
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
    size_t files_per_col = 0;
    if (files_per_line != 0)
    {
        files_per_col = files.size() / files_per_line;
    }
    else
    {
        files_per_line = 1;
        files_per_col = files.size();
    }
    
    files.resize(files_per_line * (files_per_col + 1));

    for (size_t i = 0; i < files_per_col + 1; ++i)
    {
        for (size_t j = 0; j < files_per_line; ++j)
        {
            string filename = files.at(j*(files_per_col+1) + i);
            if (filename != "")
            {
                cout << left << setw(max_width + 1) << filename;
            }
        }
        cout << endl;
    }
    
    
}

void add_directory(vector<DirNode> &dirvect, string d, bool all_flag) 
{ 
    DirNode dn(d);
    list_dir(d, dn.files, all_flag);
    dirvect.push_back(dn);
}

void recurse_directory(vector<DirNode> &dirvect, string d, bool all_flag)
{
    DirNode root(d);
    list_dir(d, root.files, all_flag);
    dirvect.push_back(root);

    for (auto subd : root.files)
    {
        string subd_full = d + "/" + subd;
        if (is_dir(subd_full) && (subd != ".." && subd != "."))
        {
            recurse_directory(dirvect, subd_full, all_flag);
        }
    }
}

void print_directories(vector<DirNode> const &dirvect, bool long_flag)
{
    for (auto &d : dirvect)
    {
        cout << endl;
        cout << d.dirname << ":" << endl;
        print_files(d.files, long_flag);
    }
}

int main(int argc, char **argv)
{
    bool long_flag = false;
    bool recursive_flag = false;
    bool all_flag = false;

    vector<string> files;// = {"a", "b", "ccc", "dddddddd", "eeeee", "f", "g", "h", "i", "j"};
    vector<string> directories;
    
    parse_arguments(argc, argv, 
                    long_flag, recursive_flag, all_flag,
                    files, directories);

    if (files.size() > 0)
    {
        //TODO check if file exists
        print_files(files, long_flag);
    }
    else if (directories.size() == 1 && !recursive_flag)
    {
        list_dir(directories.at(0), files, all_flag);
        print_files(files, long_flag);
        return 0; 
    }
    else if (directories.size() == 0)
    {
        if (!recursive_flag)
        {
            list_dir(".", files, all_flag);
            print_files(files, long_flag);
        }
        else
        {
            directories.push_back(".");       
        }
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
