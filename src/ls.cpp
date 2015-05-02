#include "ANSI.h"
#include <algorithm>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <stddef.h>
#include <dirent.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <sstream>

using namespace std;

struct DirNode
{
    string dirname;
    vector<string> files;
    DirNode(string dirname) : dirname(dirname) {}
    bool operator<(DirNode const &other) const
    {
        string a = dirname;
        string b = other.dirname;
        boost::to_upper(a);
        boost::to_upper(b);
        return a < b;
    }
};

/*
 *
 *  helper functions
 *
 *
 */

//returns whether the file *resembles* a directory
string base_name(string const &file)
{
    istringstream ss(file);
    string tok;

    while (getline(ss, tok, '/'));

    return tok;
}

bool is_dir(string const &file)
{
    struct stat s;
    if (stat(file.c_str(), &s) == -1)
    {
        perror("stat");
        return false;
    }
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
            files.push_back(d + "/" + curr_name);
        }
    }

    delete prev;
}

string get_permissions(mode_t m)
{

    string perms = "-rwxrwxrwx";
    
    //set type
    if (S_ISDIR(m)) perms.at(0) = 'd';
    if (S_ISBLK(m)) perms.at(0) = 'b';
    if (S_ISCHR(m)) perms.at(0) = 'c';
    if (S_ISLNK(m)) perms.at(0) = 'l';

    //set permissions
    for (int i = 0; i < 9; ++i)
    {
        if (((0400 >> i) & m) == 0) perms.at(i+1) = '-';
    }

    return perms;

}

string get_user(uid_t uid)
{
    auto *pw = getpwuid(uid);
    if (pw == NULL)
    {
        perror("getpwuid");
        return "ERROR";     
    }
    string ret = pw->pw_name;
    return ret;
}

string get_group(gid_t gid)
{
    auto *g = getgrgid(gid);
    if (g == NULL)
    {
        perror("getgrgid");
        return "ERROR";     
    }
    string ret = g->gr_name;
    return ret;
}

struct stat_string
{
    string permissions;
    string link_count;
    string user;
    string group;
    string size;
    string date;
    //string time_or_year;
    string name;
    size_t block_count;
};

string get_date(time_t t)
{
    char c_str_date[256];
    struct tm *time = localtime(&t);
    strftime(c_str_date, 256, "%c", time);
    return c_str_date;
}

/*string get_time_or_year()
{
    char[256] c_str_date;
    struct time = localtime(t);
    time_t now = time(0);
    if (now - t >= 60*60*24*365)
    {
        strftime(c_str_date, 256, "%Y", &time);
    }
    else
    {
        strftime(c_str_date, 256, "", &time);
    }
    return c_str_date;
}
*/
void get_stats(vector<string> const &files, vector<stat_string> &stats)
{
    for (size_t i = 0; i < files.size(); ++i)
    {
        stat_string ss;
        ss.name = files.at(i);
        struct stat s;
        if (stat(files.at(i).c_str(), &s) == -1)
        {
            perror("stat");
            continue;
        }
        else
        {
            ss.permissions = get_permissions(s.st_mode);
            ss.link_count = to_string(s.st_nlink);
            ss.user = get_user(s.st_uid);
            ss.group = get_group(s.st_gid);
            ss.size = to_string(s.st_size);
            ss.date = get_date(s.st_mtime);
            ss.block_count = s.st_blocks;
        }
        stats.push_back(ss);
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


bool string_lt(string const &a_, string const &b_)
{
    string a = a_;
    string b = b_;
    boost::to_upper(a);
    boost::to_upper(b);
    return a < b;
}

bool is_exec(string const &file)
{
    struct stat s;
    if (stat(file.c_str(), &s) == -1)
    {
        perror("stat");
        return false;
    }
 
    return ((S_IXUSR | S_IXGRP | S_IXOTH) & s.st_mode) != 0;     
}

void print_colored(string const &file, ssize_t w = 0)
{
    
    string f = base_name(file);
    
    if (is_dir(file))
    {
        setFG(CYAN);
    }
    else if (is_exec(file))
    {
        setFG(GREEN);
    }
    if (f.at(0) == '.')
    {
        setBG(BLACK);
    }

    ssize_t pad_amt = max(static_cast<ssize_t>(0), w - static_cast<ssize_t>(f.size()));
    string padding(pad_amt,' ');
    cout << f;
    resetColor();
    cout << padding;
}

//TODO: implement long_flag
void print_files(vector<string> files, bool long_flag)
{
    sort(files.begin(), files.end(), string_lt);

    if (long_flag)
    {
        vector<stat_string> stats;
        get_stats(files, stats);

        size_t l_user = 0;
        size_t l_group = 0;
        size_t l_size = 0;
        size_t l_link = 0;
        for (auto const &s: stats)
        {
            l_user = max(l_user, s.user.size());
            l_group = max(l_group, s.group.size());
            l_size = max(l_size, s.size.size());
            l_link = max(l_link, s.link_count.size());
        }

        for (auto const &s : stats)
        {
            cout << s.permissions << " "
                 << setw(l_link) << s.link_count << " "
                 << setw(l_user) << s.user << " "
                 << setw(l_group) << s.group << " "
                 << setw(l_size) << s.size << " "
                 << s.date << " ";
            print_colored(s.name);
            cout << endl;
        }
    }           
    else
    {

        size_t total_width = 80;
        size_t max_width = 0;
        for (auto &s : files)
        {
            if (base_name(s).size() > max_width)
            {
                max_width = base_name(s).size();
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
                    print_colored(filename, max_width + 1);
                }
            }
            cout << endl;
        }
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

    for (auto subdir : root.files)
    {
        string subd = base_name(subdir);
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
