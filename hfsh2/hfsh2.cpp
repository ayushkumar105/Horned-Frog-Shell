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
// Structs and global variables.
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
void executeBuiltCommand(const Command& command);
int executeCommands(vector<Command>& commands);
int batchFile(char* filename);
string isPathValid(const vector<string>& paths, const Command& comm);
int checkForRedirect(Command& comm);
int parseCommand(const Command& comm, char* args[], string& commPath, string& path);


//*********************************************************
//
// Function Definitions
//
//*********************************************************


/****************************************************************************************************************
*   Executes all the commands by creating a child process for each command that then executes the shell function.
*   Then it waits for each of the child processes created by it. 
*   Or if it is a built-in command then it calls the executeCommand function to handle those commands.
*****************************************************************************************************************/
int executeCommands(vector<Command>& commands)
{
    //Create an array of PID for each command.
    pid_t pid[commands.size()];
    int childNum = 0;

    //Iterate through the command vector to either create a child to run a command or just run a built-in comamnd. 
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
        }
        else
        {
            ++childNum;
            executeBuiltCommand(commands[i]);
        }
    }
    for(int i = 0; i< commands.size() - childNum; ++i)
    { 
        waitpid(pid[i], NULL, 0);
    }
    
    
    return 0;
}

//Handles all the built-in commands. 
void executeBuiltCommand(const Command& command)
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
    }
}

//Check if the command is in the built-in command vector. 
bool checkBuiltIn(const Command& command)
{
    if(find(built_in.begin(), built_in.end(), command.comm) != built_in.end())
    {
        return true;
    }
    return false;
}


/************************************************************
*   Check if any one of the paths are valid executable files. 
*   It return "-1" if none of the paths are valid.  
*   Else it return the valid path string.
*************************************************************/
string isPathValid(const vector<string>& paths, const Command& comm)
{
    bool isValid =false;
    string path = paths.front() + "/" + comm.comm;
    int i = 0;
    
    if (access(path.c_str(), X_OK) == -1)
    {
        
        while (i < paths.size())
        {
            path = paths[i] + "/" + comm.comm;
            
            if(access(path.c_str(), X_OK) == 0)
            {
                break;
            }
            ++i;
        }
        
    }
    if (access(path.c_str(), X_OK) == -1)
    {
        
        isValid = false;
    }
    else
    {
        isValid = true;
    }
    if(isValid)
    {
        return path;
    }
    
    return "-1";
    
}


int parseCommand(const Command& comm, char* args[], string& commPath, string& path)
{
    //Parsing command based on the number of arguments. This is not the final parsing that needs to be done!
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
    
    
    return 0;
}


/**********************************************************************************************************
*   Check if the command includes a redirection option. 
*   If redirect is 1 then there was an error with the command entered, if it was -1 then no redirect needed. 
*   If 0 then redirection has been done.
***********************************************************************************************************/
int checkForRedirect(Command& comm)
{
    //Check if '>' exists in our argument list.
    if(find(comm.args.begin(), comm.args.end(), ">") != comm.args.end())
    {
        
        int countOfOp = 0;
        int idx = 0;

        //Count the number of '>' characters. Get the index of the last '>' character
        for(int i = 0; i<comm.args.size(); ++i)
        {
            if (comm.args[i] == ">")
            {
                countOfOp++;
                idx = i;
            }
        }

        //If more than one '>' characters exist or '>' is not the second last argument element then it is an error. 
        if(countOfOp > 1 || idx != comm.args.size() - 2)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1; 
        }
        
        //Open file with O_TRUNC option to truncate the file. 
        int file = Open(comm.args.back().c_str(), O_TRUNC|O_WRONLY|O_CREAT, 0600);
        
        //Closing the file after redirection as it is a redundant file descriptor after redirection. 
        Dup2(file, 1);
        Close(file);

        //Removing the redirection argument as the direction has been done. 
        comm.args.pop_back();
        comm.args.pop_back();
    }
    else
    {
        return -1;
    }
    return 0;
}

int shell(Command& comm)
{

    //Save the original STDOUT
    int savedStdOut = dup(1);

    //Check if path is empty. 
    if(currPaths.empty())
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1; 
    }

    /*
    *   Check if the command includes a redirection option. 
    *   If redirect is 1 then there was an error with the command entered, if it was -1 then no redirect needed. 
    *   If 0 then redirection has been done.
    */
    int redirect = checkForRedirect(comm);
    if(redirect == 1)
    {
        return 1;
    }
    
    
    //Checking if the any one of the paths in current path array is valid.
    string path;
    if ((path = isPathValid(currPaths, comm)) == "-1")
    {   
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    }
    
    //Parse the command so that we can use it for execvp function. 
    string commPath = comm.comm;
    char *args[256];
    if(parseCommand(comm, args, commPath, path) == 1)
    {
        return 1;
    }

    //Check if the command is a bash script. If it is then change args and commPath to match the bash script parameters for execvp. 
    if(comm.comm.size() > 3 && (comm.comm.substr(comm.comm.size() - 3) == ".sh"))
    {
        
        if (access(path.c_str(), X_OK) == -1)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1; 
        }
        
        string shellArgument = "sh";
        string shellPath = path;
        commPath = "/bin/sh";
        char *arg0 =  const_cast<char*>(shellArgument.c_str());
        char *arg1 =  const_cast<char*>(shellPath.c_str());
        args[0] = arg0;
        args[1] = arg1;
        args[2] = NULL;
    }
    
    //Forking a childing process. 
    pid_t pid = Fork();  
    if(pid != 0)
    {
        /****************************
        * This is the parent process*
        *****************************/
        
        //Waiting for the child process o finish. 
        Waitpid(pid, NULL, 0);
        
        if(redirect != -1)
        {
            //Restore the usual STDOUT if it was redirected. 
            Dup2(savedStdOut, 1);
        }
        
    }
    else
    {
        /****************************
        * This is the child process*
        *****************************/

        //Calling execvp. If it returns then command failed to execute.
        if ((execvp(commPath.c_str(), args)) < 0)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }
        exit(1);
    }
    
    return 0;
}

//Batch file to process files an inputs. 
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

        if(toks[0] != NULL)
        {
            string str = toks[0];
            comm.comm = str;

            //Check if command is '&' or not.
            if(comm.comm == "&")
            {
                comm = {};
                continue;
            }

            /* simple loop to echo all arguments */
            int ii = 1;

            while(toks[ii] != NULL)
            {
                /*
                *   If we encounter '&' character then we can append the command to commands vector 
                *   and start reading the line again for another comamnd.
                */
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
            
            //Clearing commands and comm after executing the command(s) to start reading reading for another command. 
            executeCommands(commands);
            commands.clear();
            comm = {};
            if(!strcmp(toks[0], STR_EXIT))
             break;
        }
    }
    Fclose(fp);
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
        cout << "hfsh> ";
        
        while(Fgets(linestr, 256, stdin)){
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

                //Check if command is '&' or not. 
                if(comm.comm == "&")
                {
                    comm = {};
                    continue;
                }
                /* simple loop to echo all arguments */
                int ii = 1;
                while(toks[ii] != NULL){

                    /*
                    *   If we encounter '&' character then we can append the command to commands vector 
                    *   and start reading the line again for another comamnd.
                    */
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
                
                //Clearing commands and comm after executing the command(s) to start reading reading for another command. 
                executeCommands(commands);
                commands.clear();
                comm = {};
                if(strcmp(toks[0], "exit") != 0)
                {
                    cout << "hfsh> ";
                }
                if(!strcmp(toks[0], STR_EXIT))
                break;
            }
        }

        /* return to calling environment */
        return( retval );
    }
    
    
}