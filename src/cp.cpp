#include <iostream>
#include <fstream>
#include <ostream>
#include "timer.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
int fileDescriptR = 0, fileDescriptW = 0;
struct stat s;
ifstream infile;
ofstream outfile;

void ReadWrite(int argc, char* argv[],int buffsize) {
    //int n = 1;
    int n = buffsize;
    char buffer[BUFSIZ];
    
    fileDescriptR = open(argv[1], O_RDONLY);
    fileDescriptW = open(argv[2], (O_WRONLY | O_CREAT), (S_IWUSR | S_IRUSR));
    
    if(fileDescriptR == -1) {
        perror("Read file could not be opened");
        exit(0);
    } 
    if(fileDescriptW == -1) {
        perror("Write file could not be opened");
        exit(0);
    }

    ssize_t bytesWritten = 0; 
    while((bytesWritten = read(fileDescriptR, buffer, n)) != 0) {
        if(bytesWritten == -1) {
            perror("Could not write to file");
            exit(0);
        }
        if(write(fileDescriptW, buffer, bytesWritten) == -1) {
            perror("Could not write to file");
            exit(0);
        }
    }
    if (close(fileDescriptR) == -1)
    {
        perror("close");   
    }
    if (close(fileDescriptW) == -1)
    {
        perror("close");
    }
}
 
 void FileStream() {
     char c;

    while(infile.get(c)) {
        outfile.put(c);
    }

     
}

int main(int argc, char* argv[]) {
    Timer t;
    double wall = 0.0, user = 0.0, system = 0.0;

    if(stat(argv[2],&s) != -1) { 
        cerr << "The file to be written to already exists" << endl;
        perror("stat");
        exit(0);
    }

    
    infile.open(argv[1]);
    outfile.open(argv[2]);
    
    t.start();
    FileStream();
    t.elapsedTime(wall,user,system);
    cout << "Get/Put Algorithm: "  << endl;
    cout << "Wall clock: " << wall << endl;
    cout << "User Time: " << user << endl;
    cout << "System Time: " << system << endl << endl;
    
    t.start();
    ReadWrite(argc, argv, 1);
    t.elapsedTime(wall,user,system);
    cout << "Read/Write Algorithm, Size 1: "  << endl;
    cout << "Wall clock: " << wall << endl;
    cout << "User Time: " << user << endl;
    cout << "System Time: " << system << endl << endl;

    t.start();
    ReadWrite(argc, argv, BUFSIZ);
    t.elapsedTime(wall,user,system);
    cout << "Read/Write Algorithm, Size BUFSIZ: "  << endl;
    cout << "Wall clock: " << wall << endl;
    cout << "User Time: " << user << endl;
    cout << "System Time: " << system << endl;

    infile.close();
    outfile.close();

    /*Timer t;
    double eTime;
    t.start();
    for (int i=0, j; i<1000000000; i++)
        j++;
    t.elapsedUserTime(eTime);
    cout << eTime << endl;*/

    return 0;
}
