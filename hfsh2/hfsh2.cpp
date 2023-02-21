//************************************************************************************************************************************************************
//
// Ayush kumar
// Operating Systems
// Programming Project #2: Writing your own shell
// February 20, 2023
// Instructor: Dr. Michael Scherger
//
//************************************************************************************************************************************************************
using namespace std;

//************************************************************************************************************************************************************
//
// Includes and Defines
//
//************************************************************************************************************************************************************
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
#define STR_CD "cd"
#define STR_PATH "path"
#define STR_BIN "/bin"

//************************************************************************************************************************************************************
//
// Extern Declarations
//
//************************************************************************************************************************************************************
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

//************************************************************************************************************************************************************
//
// Structs and global variables.
//
//************************************************************************************************************************************************************
struct command{
    string comm;
    vector<string> args;
};
typedef command Command;
vector<string> currPaths = {STR_BIN};
vector<string> built_in = {STR_CD, STR_PATH, STR_EXIT};

//************************************************************************************************************************************************************
//
// Function Prototypes
//
//************************************************************************************************************************************************************
int shell(Command& comm);
bool checkBuiltIn(const Command& command);
void executeBuiltCommand(const Command& command);
void executeCommands(vector<Command>& commands);
int batchFile(char* filename);
string isPathValid(const vector<string>& paths, const Command& comm);
int checkForRedirect(Command& comm);
void parseCommand(const Command& comm, char* args[], string& commPath, string& path);


//************************************************************************************************************************************************************
//
// Function Definitions
//
//************************************************************************************************************************************************************

//************************************************************************************************************************************************************
//
// Execute Commands Function
//
// Executes all the commands by creating a child process for each command that then executes the shell function.
// Then it waits for each of the child processes created by it to exit. 
// Or if it is a built-in command then it calls the executeCommand function to handle those commands without forking a child.
//
// Return Value
// ------------
// void
//
// Function Parameters
// -------------------
// commands   vector<Command>   reference   The command vector
//
// Local Variables
// ---------------
// pid        pid_t[]           Array of PIDs for each child process
// childNum   int               Number of child processes that will be created
//
//************************************************************************************************************************************************************
void executeCommands(vector<Command>& commands)
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

    //Wait for each child process
    for(int i = 0; i< commands.size() - childNum; ++i)
    { 
        waitpid(pid[i], NULL, 0);
    }
}

//************************************************************************************************************************************************************
//
// Execute Built-In Command Function
//
// Handles all the built-in commands
//
// Return Value
// ------------
// void
//
// Function Parameters
// -------------------
// command   Command   reference   The command to be executed
//
// Local Variables
// ---------------
// i         string    Iterator for the currPaths vector
//
//************************************************************************************************************************************************************
void executeBuiltCommand(const Command& command)
{
    
    if (command.comm == STR_EXIT)
    {
        //If command is not empty then print the error message
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
    else if (command.comm == STR_CD)
    {
        //If command has no argument then print error message
        if (command.args.size() > 1 || command.args.size() == 0)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else
        {
            //If chdir fails print the error message
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
            //Add the path arguments to the currPaths vector
            currPaths.push_back(i);
        }
    }
}

//************************************************************************************************************************************************************
//
// Check Built-In Function
//
// Check if the command is in the built-in command vector. 
//
// Return Value
// ------------
// bool True/False if the Command is built-in
//
// Function Parameters
// -------------------
// command   Command   reference   The command to be checked
//
// Local Variables
// ---------------
// No local variables
//
//************************************************************************************************************************************************************
bool checkBuiltIn(const Command& command)
{
    //Check built_in vector for the command
    if(find(built_in.begin(), built_in.end(), command.comm) != built_in.end())
    {
        return true;
    }
    return false;
}

//************************************************************************************************************************************************************
//
// Check Valid Paths Function
//
// Check if any one of the paths are valid executable files. 
//
// Return Value
// ------------
// string   "-1"/<Path String>   "-1" if no valid path else first valid path string 
//
// Function Parameters
// -------------------
// paths    vector<string>   reference   Paths vector
// commands Command          reference   The command 
//
// Local Variables
// ---------------
// isValid   bool            Keep track of any valid paths available
// path      string          A string variable to keep track of the exact path that needs to be searched
// i         int             Iterator variable to iterate through all strings in paths vector 
//
//************************************************************************************************************************************************************
string isPathValid(const vector<string>& paths, const Command& comm)
{
    //Local variables
    bool isValid =false;
    string path = paths.front() + "/" + comm.comm;
    int i = 0;
    
    //Check the initial path
    if (access(path.c_str(), X_OK) == -1)
    {
        
        //Keep checking until a valid path is found
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

    //Check if we existed the loop with a valid path
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


//************************************************************************************************************************************************************
//
// Parse Command Function
//
// Initial parsing of commands before checking for bash script executable command
//
// Return Value
// ------------
// void
//
// Function Parameters
// -------------------
// commPath     string       reference   Path for the command to be added to the final command
// comm         Command      reference   The command 
// args         (char *)[]   value       An array of strings of arguments to pass to execvp  
//
// Local Variables
// ---------------
// i            int          Iterator variable to iterate through all strings in command argument vector 
//
//************************************************************************************************************************************************************
void parseCommand(const Command& comm, char* args[], string& commPath)
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
}

//************************************************************************************************************************************************************
//
// Check for redirect Function
//
// Check if the command includes a redirection option and if it does check arguments for validilty of the output file
//
// Return Value
// ------------
// int   1/-1/0   If redirect is 1 then there was an error with the command entered, if it was -1 then no redirect needed. If 0 then redirection has been done.
//   
//
// Function Parameters
// -------------------
// commands   Command   reference   The command 
//
// Local Variables
// ---------------
// coutOfOp   int       Count of number of ">" operators in the args vector of the command
// idx        int       Keep track of the index of the ">" operator
// i          int       Iterator variable to iterate through all strings in command argument vector
// file       int       File descriptor of the redirected output destination
// err        int       File descriptor of the redirected error destination
//
//**************************************************************************************************************************************************************
int checkForRedirect(Command& comm)
{
    //Check if '>' exists in our argument list.
    if(find(comm.args.begin(), comm.args.end(), ">") != comm.args.end())
    {
        //Local Variables
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
        int err = Open(comm.args.back().c_str(), O_TRUNC|O_WRONLY|O_CREAT, 0600);

        //Closing the file after redirection as it is a redundant file descriptor after redirection.
        Dup2(file, STDERR_FILENO);
        fflush(stdout); Close(file);
        Dup2(err, STDOUT_FILENO);
        fflush(stderr); Close(err); 
        

        //Removing the redirection argument as the redirection has been done. 
        comm.args.pop_back();
        comm.args.pop_back();
    }
    else
    {
        //No redirection needed
        return -1;
    }
    //redirection successful 
    return 0;
}

//************************************************************************************************************************************************************
//
// Shell Function
//
// Executes all commands except for built-in commands
//
// Return Value
// ------------
// int   1/0   1 if an error occured else 0 if command runs successfully
//   
//
// Function Parameters
// -------------------
// commands         Command     reference   The command 
//
// Local Variables
// ---------------
// savedStdOut      int         Save the stdout reference incase redirection is needed
// saveStdErr       int         Save the stderr reference incase redirection is needed
// redirect         int         If redirect is needed then it return 0 on successful redirect and 1 on unsuccessful redirect and -1 for no redirection
// path             string      Has the full path with command appended to it, "-1" if no valid path exists
// commPath         string      Has just the command string without arguments 
// args             (char *)[]  An array of arguments to be passed to execvp
// shellArguments   string      String literal "Sh"
// arg0             char*       Used for conversion from string to cstring
// arg1             char*       Used for conversion from string to cstring
//
//**************************************************************************************************************************************************************
int shell(Command& comm)
{

    //Save the original STDOUT/STDERR
    int savedStdOut = dup(STDOUT_FILENO);
    int savedStdErr = dup(STDERR_FILENO);

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
    parseCommand(comm, args, commPath);
    

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
        commPath = "/bin/sh";
        char *arg0 =  const_cast<char*>(shellArgument.c_str());
        char *arg1 =  const_cast<char*>(path.c_str());
        args[0] = arg0;
        args[1] = arg1;
        args[2] = NULL;
    }
    
    //Forking a childing process. 
    pid_t pid = Fork();  
    if(pid != 0)
    {
        /*****************************
        * This is the parent process *
        ******************************/
        
        //Waiting for the child process to finish. 
        Waitpid(pid, NULL, 0);
        
        if(redirect != -1)
        {
            //Restore the usual STDOUT/STDERR if they were redirected. 
            Dup2(savedStdOut, STDOUT_FILENO);
            Dup2(savedStdErr, STDERR_FILENO);
        }
        //Close the saved std out/err as they will be restored if they were redirected
        Close(savedStdOut);
        Close(savedStdErr);
    }
    else
    {
        /****************************
        * This is the child process *
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

//************************************************************************************************************************************************************
//
// Batch File Function
//
// Batch file function to process all inputs in the specified file
//
// Return Value
// ------------
// int   1/0   1 if an error occured else 0 if file was successfully parsed
//   
//
// Function Parameters
// -------------------
// filename   char*             value   Name of the file to be parsed
//
// Local Variables
// ---------------
// fp         FILE*             point to the file specified
// commands   vector<Command>   A vector of all the commands in the file
// comm       Command           A variable to help parse the commands in the file
// ii         int               Used in the parsing code 
// toks       char**            Used in the parsing code
// buffer     YY_BUFFER_STATE   Used in the parsing code
// retval     int               Used in the parsing code 
// linestr    char[]            Used in the parsing code 
//
//************************************************************************************************************************************************************** 
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
    Command comm;

    /* initialize local variables */
    ii = 0;
    toks = NULL;
    retval = 0;
    comm = {};

    
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
            
            //Clearing commands and comm after executing the command(s) to start reading for another command. 
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


//************************************************************************************************************************************************************
//
// Main Function
//
// Main function to process all inputs from the terminal. If a batch file is specified then batchFile function is called to parse the file.
//
// Return Value
// ------------
// int   1/0   1 if an error occured else 0 if file was successfully parsed
//   
//
// Function Parameters
// -------------------
// argc       int               value       number of arguments provided while running the program 
// argv       (char *)[]        value       cstring array of arguments provided in the terminal 
//
// Local Variables
// ---------------
// commands   vector<Command>   A vector of all the commands provided in the terminal
// comm       Command           A variable to help parse the commands in the file
// ii         int               Used in the parsing code 
// toks       char**            Used in the parsing code
// buffer     YY_BUFFER_STATE   Used in the parsing code
// retval     int               Used in the parsing code 
// linestr    char[]            Used in the parsing code 
//
//************************************************************************************************************************************************************** 
int main(int argc, char *argv[])
{
    //Check if a batch file was specified
    if(argc > 1)
    {
        //If there are more than one arguments then print the error message and exit
        if(argc > 2)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }

        // batchFile returns 1 on error while opening the file and then we can exit with rc 1
        if (batchFile(argv[1]) == 1)
        {
            exit(1);
        }
        else
        {
            exit(0);
        }
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
        Command comm;

        /* initialize local variables */
        ii = 0;
        toks = NULL;
        retval = 0;
        comm = {};

        
        /* main loop */

        //Printing the first prompt
        cout << "hfsh2> ";
        
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
                if(strcmp(toks[0], STR_EXIT) != 0)
                {
                    //Printing the prompt
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