//*********************************************************
//
// PUT YOUR NAME HERE!!!!
// Operating Systems
// Writing Your Own Shell: hfsh2
//
//*********************************************************


//*********************************************************
//
// Includes and Defines
//
//*********************************************************
#include <cstdio>
#include <cstdlib>
#include <cstring>

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


//*********************************************************
//
// Namespace
//
// If you are new to C++, uncommment the `using namespace std` line.
// It will make your life a little easier.  However, all true
// C++ developers will leave this commented.  ;^)
//
//*********************************************************
// using namespace std;


//*********************************************************
//
// Function Prototypes
//
//*********************************************************


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

    /* initialize local variables */
    ii = 0;
    toks = NULL;
    retval = 0;

    /* main loop */
    while(fgets(linestr, 256, stdin)){
        // make sure line has a '\n' at the end of it
        if(!strstr(linestr, "\n"))
            strcat(linestr, "\n");

        /* get arguments */
        buffer = yy_scan_string(linestr);
        yy_switch_to_buffer(buffer);
        toks = gettoks();
        yy_delete_buffer(buffer);

        if(toks[0] != NULL){
            /* simple loop to echo all arguments */
            for(ii=0; toks[ii] != NULL; ii++){
                printf( "Argument %d: %s\n", ii, toks[ii] );
            }

            if(!strcmp(toks[0], STR_EXIT))
            break;
        }
    }

    /* return to calling environment */
    return( retval );
}