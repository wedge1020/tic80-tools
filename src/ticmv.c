#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <getopt.h>
#include "tictools.h"

void    display_help (char *);
void    display_version (void);

int main(int argc, char **argv)
{
	Chunk *dest                          = NULL;
	Chunk *tmp                           = NULL;
    FILE  *in                            = NULL;
    int    index                         = 0;
    int    count                         = 0;
    int    offset                        = 0;
    int    flags                         = FLAG_DEFAULT;  // getopt
    int    opt                           = 0;             // getopt
    int    space                         = -12;
    int    indent                        = 1;
	List  *source                        = NULL;
    ussi   bank                          = 0;
    ussi   banknum                       = 0xFF;          // bitflag
    ussi   byte                          = 0;
	ussi   place                         = 0;
    ussi   plural                        = 0;
	ussi  *data                          = NULL;
    ussi   type                          = 0;
    usi    size                          = 0;
    usi    word                          = 0;

    opt                                  = getopt (argc, argv, "fhiVv");
    while (opt                          != -1)
    {
        switch (opt)
        {
            case 'f':
                flags                    = flags & ~FLAG_PROMPT;
                break;

            case 'h':
				display_help (argv[0]);
                break;

            case 'i':
                flags                    = flags | FLAG_PROMPT;
                break;

            case 'V':
				display_version ();
                break;

            case 'v':
				flags                    = flags | FLAG_VERBOSE;
                break;

            default: /* '?' */
                fprintf (stderr, "Usage: %s [-b #] [-1hlV] name\n", argv[0]);
                exit (EXIT_FAILURE);
        }
        opt                              = getopt (argc, argv, "fhiVv");
    }

	//////////////////////////////////////////////////////////////////////////
	//
	// Ensure there are enough files to perform the transaction: two or more
	//
    plural                               = (argc - optind);
	if (plural                          == 0)
	{
		fprintf (stderr, "%s: missing file operand\n",          argv[0]);
		fprintf (stderr, "Try '%s -h' for more information.\n", argv[0]);
		exit (1);
	}
	else if (plural                     <  2)
	{
		fprintf (stderr, "%s: missing destination ",            argv[0]);
		fprintf (stderr, "file operand after '%s'\n",           argv[optind]);
		fprintf (stderr, "Try '%s -h' for more information.\n", argv[0]);
		exit (1);
	}
	
	//////////////////////////////////////////////////////////////////////////
	//
	// Create the list of source files
	//
	source                               = (List *) malloc (sizeof (List));
	source -> start                      = NULL;
	source -> next                       = NULL;
	tmp                                  = source -> start;
	if (source                          == NULL)
	{
		fprintf (stderr, "ERROR: could not allocate storage for source\n");
		exit (1);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// Process for each file specified on command-line
	//
    for (index = optind; index < argc - 1; index++)
    {
		if (tmp                         == NULL)
		{
			tmp                          = inventory (argv[index]);
		}
		else
		{
			tmp -> next                  = inventory (argv[index]);
			tmp                          = tmp -> next;
		}

		if (tmp                         == NULL)
		{
			fprintf (stderr, "%s: cannot locate '%s': ", argv[0], argv[index]);
			fprintf (stderr, "No such file or directory\n");
			exit (1);
		}
    }
	
	//////////////////////////////////////////////////////////////////////////
	//
	// Itemize the destination assets (if it exists)
	//
	dest                                 = inventory (argv[argc - 1]);
	if (dest                            == NULL)
	{
		
	}

    return (0);
}

void    display_help(char *argv)
{
	fprintf (stdout, "Usage: %s [OPTION].. [[BANK#]:[CHUNK]]SOURCEFILE [[BANK#]:[CHUNK]]DESTFILE\n\n", argv);
	fprintf (stdout, " -f    force operation (no prompting)\n");
	fprintf (stdout, " -h    show this help\n");
	fprintf (stdout, " -i    interactive, prompt before overwrite\n");
	fprintf (stdout, " -v    verbose operation\n");
	fprintf (stdout, " -V    show version information\n");
	fprintf (stdout, "\nNOTE: Source and destination CAN be the same file\n");
	exit (0);
}

void    display_version(void)
{
	fprintf (stdout, "tictools mv (tmv/ticmv) release 20230217-160715\n");
	exit (0);
}

//////////////////////////////////////////////////////////////////////////////
//
// Collect (in a list) an inventory of TIC cartridge chunks
//
Chunk *inventory (char *argv)
{
	//////////////////////////////////////////////////////////////////////////
	//
	// Declare variables
	//
	Chunk *start                         = NULL;
	Chunk *tmp                           = NULL;
    FILE  *in                            = NULL;
    int    offset                        = 0;
    usi    size                          = 0;
    usi    word                          = 0;
    ussi   byte                          = 0;

	//////////////////////////////////////////////////////////////////////////
	//
	// Create the first node of the chunk list, only proceed if success
	//
	start                                = (Chunk *) malloc (sizeof (Chunk));
	tmp                                  = start;
	if (start                           != NULL)
	{
		//////////////////////////////////////////////////////////////////////
		//
		// Process file (only proceed if success in opening)
		//
		in                               = fopen (argv, "r");
		while ((in                      != NULL) &&
			   (tmp                     != NULL) &&
			   (!feof (in)))
		{
			//////////////////////////////////////////////////////////////////
			//
			// Read first byte: the bank/type byte
			//
			tmp -> offset                = offset;
			byte                         = fgetc (in);
            offset                       = offset + 1; // count the byte

            //////////////////////////////////////////////////////////////////
            //
            // Separate the bank/type byte
            //
            tmp -> bank                  = byte & 0xE0;
            tmp -> bank                  = tmp -> bank >> 5;
			tmp -> type                  = byte & 0x1F;

            //////////////////////////////////////////////////////////////////
            //
            // Read and de-endiafy the size bytes
            //
            size                         = fgetc (in); offset++;
            byte                         = fgetc (in); offset++;
            word                         = byte;
            word                         = word << 8;
            size                         = size | word;
			tmp -> size                  = size;
            byte                         = fgetc (in); offset++; // reserved
            tmp -> reserved              = byte;

			//////////////////////////////////////////////////////////////////
			//
			// Allocate, read and store chunk data into current node
			//
			size                         = sizeof (ussi) * tmp -> size;
			tmp -> data                  = (ussi *) malloc (size);
			if (tmp -> data             != NULL)
			{
				fread (tmp -> data, 1, tmp -> size, in);
				offset                   = offset + size;
			}

			//////////////////////////////////////////////////////////////////
			//
			// Create the next node of the chunk list
			//
			tmp -> next                  = (Chunk *) malloc (sizeof (Chunk));
			tmp                          = tmp -> next;
        }
        fclose (in);
    }
    return (start);
}

//////////////////////////////////////////////////////////////////////////////
//
// Merge different chunk lists into one
//
Chunk *mergechunks (List *source)
{
	Chunk *ctmp                          = NULL;
	Chunk *mtmp                          = NULL;
	List  *ltmp                          = NULL;

	ltmp                                 = source;
	mtmp                                 = ltmp -> start;
	while (ltmp                         != NULL)
	{
		while (mtmp -> next             != NULL)
		{
			mtmp                         = mtmp -> next;
		}

		mtmp -> next                     = ltmp -> next -> start;
			if (mtmp                    == NULL)
			{
				mtmp                     = ctmp;
			}
			else
			{
				mtmp -> next             = 
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// Synchronize in-memory data to file on disk
//
Chunk *sync2file (char *argv, List *source)
{

}
