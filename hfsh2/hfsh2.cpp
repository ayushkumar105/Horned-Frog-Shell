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
vector<string> currPaths = {"/bin"};
vector<string> built_in = {"cd", "path", "exit"};

//*********************************************************
//
// Function Prototypes
//
//*********************************************************
int shell(Command& comm);
bool checkBuiltIn(const Command& command);
string parseInput(const string& input);
void executeCommand(const Command& command);
int executeCommands(vector<Command>& commands);


//*********************************************************
//
// Function Definitions
//
//*********************************************************
int executeCommands(vector<Command>& commands)
{
    pid_t pid[commands.size()];
    for(int i =0; i< commands.size(); ++i)
    {
        if(!(checkBuiltIn(commands[i])))
        {
            pid[i] = Fork();
            
            if(pid[i] == 0)
            {
                shell(commands[i]);
                exit(0);
            }
            // if(pid != 0)
            // {
            //     waitpid(pid, NULL, 0);
            // }
        }
        else
        {
            executeCommand(commands[i]);
        }
    }
    for(int i = 0; i< commands.size(); ++i)
    { 
        waitpid(pid[i], NULL, 0);
    }
    
    
    return 0;
}


void executeCommand(const Command& command)
{
    if (command.comm == "exit")
    {
        if(!command.args.empty())
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }
        else
        {
            exit(0);
        }
        
    }
    else if (command.comm == "cd")
    {
        if (command.args.size() > 1 || command.args.size() == 0)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else
        {
            if(chdir(command.args[0].c_str()) != 0)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message)); 
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
        // for(auto i : currPaths)
        // {
        //     cout << "For execute: " << i << " ";
        // }
        // cout << endl;
        
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

int shell(Command& comm)
{
    string file;
    if(currPaths.empty())
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 0; 
    }
    if(find(comm.args.begin(), comm.args.end(), ">") != comm.args.end())
    {
        
        int countOfOp = 0;
        int idx = 0;
        for(int i = 0; i<comm.args.size(); ++i)
        {
            if (comm.args[i] == ">")
            {
                countOfOp++;
                idx = i;
            }
        }
        //cout << comm.args[0] << endl;
        if(countOfOp > 1 || idx != comm.args.size() - 2)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 0; 
        }
        file = comm.args.back();
        Fclose(Fopen(comm.args.back().c_str(), "w"));
        int file = Open(comm.args.back().c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
        comm.args.pop_back();
        comm.args.pop_back();
        Dup2(file, 1);
    }
    // if(comm.comm.size() > 3 && (comm.comm.substr(comm.comm.size() - 3) == ".sh"))
    // {
        
    //     if (access(path.c_str(), X_OK) == -1)
    //     {
    //         char error_message[30] = "An error has occurred\n";
    //         write(STDERR_FILENO, error_message, strlen(error_message));
    //         return 0; 
    //     }
    //     string shellArgument = "sh";
    //     string shellPath = path;
    //     commPath = "/bin/sh";
    //     args[0] = const_cast<char*>(shellArgument.c_str());
    //     args[1] = const_cast<char*>(shellPath.c_str());
    //     args[2] = NULL;
    //     //cout << "hiii";
    // }
    
    string path = currPaths.front() + "/" + comm.comm;
    int i = 0;
    
    if (access(path.c_str(), X_OK) == -1)
    {
        
        while (i < currPaths.size())
        {
            //cout << currPaths[i] << endl;
            path = currPaths[i] + "/" + comm.comm;
            
            if(access(path.c_str(), X_OK) == 0)
            {
                break;
            }
            ++i;
        }
        
        if (access(path.c_str(), X_OK) == -1)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 0; 
        }
    }
    
    
    string commPath = path;
    char *args[256];
    
    if (!comm.args.empty())
    {
        args[0] = const_cast<char*>(commPath.c_str());
        for(int i = 0; i<comm.args.size(); ++i)
        {
            char *arg1 = const_cast<char*>(comm.args[i].c_str());
            args[i + 1] = arg1;
        }
        args[comm.args.size() + 1] = NULL;
    }
    else
    {
        args[0] = const_cast<char*>(commPath.c_str());
        args[1] = NULL;
    }
    // string s = comm.comm.substr(comm.comm.size() - 3);
    // //cout << (s == ".sh");
    if(comm.comm.size() > 3 && (comm.comm.substr(comm.comm.size() - 3) == ".sh"))
    {
        //cout << path;
        if (access(path.c_str(), X_OK) == -1)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 0; 
        }
        string shellArgument = "sh";
        string shellPath = path;
        commPath = "/bin/sh";
        args[0] = const_cast<char*>(shellArgument.c_str());
        args[1] = const_cast<char*>(shellPath.c_str());
        args[2] = NULL;
        //cout << "hiii";
    }
    //cout << currPaths.back();
    
    // cout << "HIII";
    // string s = "sh";
    // string s1 = "tests/p1.sh";
    // args[0] = const_cast<char*>(s.c_str());
    // args[1] = const_cast<char*>(s1.c_str());
    // args[2] = NULL;
    
    pid_t pid = Fork();
    
    if(pid != 0)
    {
        
        
        Waitpid(pid, NULL, 0);
        // if(file != "")
        // {
        //     Fclose(Fopen(file.c_str(), "w"));
        // }
        //printf("%d has exited \n", pid);
        
    }
    else
    {
        
        if ((execve(commPath.c_str(), args, NULL)) < 0)
        {
            
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }
        
        exit(0);
    }
    
    return 0;
}

int batchFile(char* filename)
{
    FILE* fp;
    
    
    // Opening file in reading mode
    fp = fopen(filename, "r");
 
    if (NULL == fp) 
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return (1);
    }
    
    /* local variables */
    int ii;
    char **toks;
    int retval;
    char linestr[256];
    YY_BUFFER_STATE buffer;
    vector<Command> commands;
    Command comm = {};

    /* initialize local variables */
    ii = 0;
    toks = NULL;
    retval = 0;
    

    
    // /* main loop */
    
    
    while(Fgets(linestr, 256, fp))
    {
        
        
        // make sure line has a '\n' at the end of it
        if(!strstr(linestr, "\n"))
            strcat(linestr, "\n");
        
         /* get arguments */
        buffer = yy_scan_string(linestr);
        yy_switch_to_buffer(buffer);
        toks = gettoks();
        yy_delete_buffer(buffer);
        if(toks[0] != NULL){
            
            string str = toks[0];
            comm.comm = str;
            /* simple loop to echo all arguments */
            int ii = 1;
            while(toks[ii] != NULL){
                //printf( "Argument %d: %s\n", ii, toks[ii] );
                if (strcmp(toks[ii], "&") == 0)
                {
                    commands.push_back(comm);
                    comm = {};
                    if (toks[ii + 1] != NULL)
                    {
                        comm.comm = toks[ii + 1];
                        ii += 2;
                    }
                    else
                    {
                        break;
                    }
                    
                }
                else
                {
                    comm.args.push_back(toks[ii]);
                    ++ii;
                }
                
            }
            if(comm.comm != "")
            {
                commands.push_back(comm);
            }
            
            // for(auto j : commands)
            // {
            //     cout << "Command: " << j.comm << " ";
            //     for(auto k : j.args)
            //     {
            //         cout << "args: " << k << " ";
            //     }
            //     cout << "\n";
            // }
            executeCommands(commands);
            commands.clear();
            comm = {};
            if(!strcmp(toks[0], STR_EXIT))
             break;
        }
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
    
    if(argc > 1)
    {
        if(argc > 2)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return (1);
        }
        return batchFile(argv[1]);
    }
    else
    {
        /* local variables */
        int ii;
        char **toks;
        int retval;
        char linestr[256];
        YY_BUFFER_STATE buffer;
        vector<Command> commands;
        Command comm = {};

        // /* initialize local variables */
        ii = 0;
        toks = NULL;
        retval = 0;
        

        
        /* main loop */
        cout << "hfsh2> ";
        
        while(Fgets(linestr, 256, stdin)){
            //currPaths.push_back("bin");
            // make sure line has a '\n' at the end of it
            if(!strstr(linestr, "\n"))
                strcat(linestr, "\n");
            
            /* get arguments */
            buffer = yy_scan_string(linestr);
            yy_switch_to_buffer(buffer);
            toks = gettoks();
            yy_delete_buffer(buffer);
            
            if(toks[0] != NULL){
                
                string str = toks[0];
                comm.comm = str;
                /* simple loop to echo all arguments */
                int ii = 1;
                while(toks[ii] != NULL){
                    //printf( "Argument %d: %s\n", ii, toks[ii] );
                    if (strcmp(toks[ii], "&") == 0)
                    {
                        commands.push_back(comm);
                        comm = {};
                        if (toks[ii + 1] != NULL)
                        {
                            comm.comm = toks[ii + 1];
                            ii += 2;
                        }
                        else
                        {
                            break;
                        }
                        
                    }
                    else
                    {
                        comm.args.push_back(toks[ii]);
                        ++ii;
                    }
                    
                }
                if(comm.comm != "")
                {
                    commands.push_back(comm);
                }
                // for(auto j : commands)
                // {
                //     cout << "Command: " << j.comm << " ";
                //     for(auto k : j.args)
                //     {
                //         cout << "args: " << k << " ";
                //     }
                //     cout << "\n";
                // }
                executeCommands(commands);
                commands.clear();
                comm = {};
                if(strcmp(toks[0], "exit") != 0)
                {
                    cout << "hfsh2> ";
                }
                if(!strcmp(toks[0], STR_EXIT))
                break;
            }
        }

        /* return to calling environment */
        return( retval );
    }
    
    
}