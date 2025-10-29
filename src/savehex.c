//
// savehex.c - a tool to read in textual representation of spaced hex bytes from
//             STDIN and to render them as raw bytes to STDOUT
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
// Pre-processor directives
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int32_t main (int32_t  argc, char **argv)
{
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Declare and initialize local variables
    //
    FILE    *out   = NULL;
    uint8_t  byte  = 0;
    
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Open the file specified by the first argument for writing
    //
    out            = fopen (argv[1], "w");
    if (out       == NULL)
    {
        fprintf (stderr, "ERROR opening '%s' for writing\n", argv[1]);
        exit (1);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Read in individual, space-separated bytes of hexadecimal data from STDIN,
    // and display them as raw data out to STDOUT
    //
    fscanf (stdin, "%hhX", &byte);
    while (!feof(stdin))
    {
        fprintf (out, "%c", byte);
        fscanf (stdin, "%hhX", &byte);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Close the open file
    //
    fclose (out);

    return (0);
}
