//*********************************************************
//
// Ayush Kumar
// Operating Systems
// Writing Your Own Shell: hfsh2
//
//*********************************************************
using namespace std;

//*********************************************************
//
// Includes and Defines
//
//*********************************************************
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<bits/stdc++.h>

#define STR_EXIT "exit"


//*********************************************************
//
// Extern Declarations
//
//*********************************************************
// Buffer state. This is used to parse string in memory...
// Leave this alone.
extern "C"{
    extern char **gettoks();
    typedef struct yy_buffer_state * YY_BUFFER_STATE;
    extern YY_BUFFER_STATE yy_scan_string(const char * str);
    extern YY_BUFFER_STATE yy_scan_buffer(char *, size_t);
    extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
    extern void yy_switch_to_buffer(YY_BUFFER_STATE buffer);
}
extern "C" { 
    #include "csapp.h"
} 

//*********************************************************
//
// Namespace
//
// If you are new to C++, uncommment the `using namespace std` line.
// It will make your life a little easier.  However, all true
// C++ developers will leave this commented.  ;^)
//
//*********************************************************
struct command{
    string comm;
    vector<string> args;
};
typedef command Command;
vector<string> currPaths;
vector<string> built_in = {"cd", "path", "exit"};

//*********************************************************
//
// Function Prototypes
//
//*********************************************************
int shell(const Command& comm);
bool checkBuiltIn(const Command& command);
string parseInput(const string& input);
void executeCommand(const string& command);

//*********************************************************
//
// Function Definitions
//
//*********************************************************
void executeCommand(const Command& command)
{
    if (command.comm == "exit")
    {
        exit(0);
    }
    else if (command.comm == "cd")
    {
        if (command.args.size() > 1)
        {
            cout << "Error \n";
        }
        else
        {
            if(chdir(command.args[0].c_str()) != 0)
            {
                cout << "Error \n";
            }
        }
    }
    else
    {
        currPaths.clear();
        for(string i : command.args)
        {
            currPaths.push_back(i);
        }
    }
}

bool checkBuiltIn(const Command& command)
{
    if(find(built_in.begin(), built_in.end(), command.comm) != built_in.end())
    {
        return true;
    }
    return false;
}

int shell(const Command& comm)
{
    
    if(!(checkBuiltIn(comm)))
    {
        string path = currPaths.front() + "/" + comm.comm;
        int i = 1;
        while (i < currPaths.size() && access("/bin/ls", X_OK) == -1)
        {
            ++i;
            path = currPaths[i] + "/" + comm.comm;
        }
        string commPath = (path).c_str();
        char *arg1 = const_cast<char*>(comm.args[0].c_str());
        char *arg2 = const_cast<char*>(comm.args[1].c_str());
        char *args[] = {const_cast<char*>(commPath.c_str()), NULL};
        // if (!comm.args.empty())
        // {
        //     cout << "hii";
        //     args[0] = const_cast<char*>(commPath.c_str());
        //     args[1] = arg1;
        //     args[2] = arg2;
        //     args[3] = NULL;
        // }
        // else
        // {
        //     args[0] = const_cast<char*>(commPath.c_str());
        //     args[1] = NULL;
        // }
        //char *args[] = {const_cast<char*>(commPath.c_str()), arg1, arg2, NULL};
        int pid, status;
        // first we fork the process
        if (pid = Fork()) {

            // pid != 0: this is the parent process (i.e. our process)
            Waitpid(pid, &status, 0); // wait for the child to exit

        } 
        else 
        {
            /* pid == 0: this is the child process. now let's load the
                "ls" program into this process and run it */

            // load it. there are more exec__ functions, try 'man 3 exec'
            // execl takes the arguments as parameters. execv takes them as an array
            // this is execl though, so:
            //      exec         argv[0]  argv[1] end
            if(false)
            {
                int file = Open("output.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
                Dup2(file, 1);
            }
            if ((execv(commPath.c_str(), args)) < 0)
            {
                cout << "Error";
            }
            /* exec does not return unless the program couldn't be started. 
                when the child process stops, the waitpid() above will return.
            */
        }
        return status; // this is the parent process again.
    }
    else
    {
        executeCommand(comm);
    }
    return 0;
}

//*********************************************************
//
// Main Function
//
//*********************************************************
int main(int argc, char *argv[])
{
    /* local variables */
    int ii;
    char **toks;
    int retval;
    char linestr[256];
    YY_BUFFER_STATE buffer;
    Command comm = {};

    /* initialize local variables */
    ii = 0;
    toks = NULL;
    retval = 0;
    

    
    /* main loop */
    cout << "hfsh> ";
    
    while(Fgets(linestr, 256, stdin)){
        cout << "hfsh> ";
        // make sure line has a '\n' at the end of it
        if(!strstr(linestr, "\n"))
            strcat(linestr, "\n");
        
        /* get arguments */
        buffer = yy_scan_string(linestr);
        yy_switch_to_buffer(buffer);
        toks = gettoks();
        yy_delete_buffer(buffer);
        
        if(toks[0] != NULL){
            currPaths.push_back("/bin");
            string str = toks[0];
            comm.comm = str;
            /* simple loop to echo all arguments */
            
            for(ii=1; toks[ii] != NULL; ii++){
                //printf( "Argument %d: %s\n", ii, toks[ii] );
                comm.args.push_back(toks[ii]);
            }

            shell(comm);
            comm = {};
            if(!strcmp(toks[0], STR_EXIT))
            break;
        }
    }

    /* return to calling environment */
    return( retval );
}