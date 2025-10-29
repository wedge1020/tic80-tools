#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <getopt.h>

#define  CHUNK_TILES         1
#define  CHUNK_SPRITES       2
#define  CHUNK_COVER_DEP     3 // deprecated as of 0.90
#define  CHUNK_MAP           4
#define  CHUNK_CODE          5
#define  CHUNK_FLAGS         6
#define  CHUNK_SAMPLES       9
#define  CHUNK_WAVEFORM     10
#define  CHUNK_PALETTE      12
#define  CHUNK_PATTERNS_DEP 13 // deprecated as of 0.80
#define  CHUNK_MUSIC        14
#define  CHUNK_PATTERNS     15
#define  CHUNK_CODE_ZIP     16 // deprecated as of 1.00
#define  CHUNK_DEFAULT      17
#define  CHUNK_SCREEN       18
#define  CHUNK_BINARY       19

#define  FLAG_DEFAULT        0
#define  FLAG_VERTICAL       1
#define  FLAG_LONG           2
#define  FLAG_BANK           4
#define  FLAG_SUMMARY        8

typedef unsigned char       uc;
typedef unsigned short int usi;
typedef unsigned int        ui;

void    display_help (char *);
void    display_version (void);

int main(int argc, char **argv)
{
    FILE *in                             = NULL;
    int   index                          = 0;
    int   count                          = 0;
    int   offset                         = 0;
    int   flags                          = FLAG_DEFAULT;  // getopt
    int   opt                            = 0;             // getopt
    int   space                          = -12;
    int   indent                         = 1;
    uc    bank                           = 0;
    uc    banknum                        = 0xFF;          // bitflag
    uc    byte                           = 0;
	uc    header                         = 0;
    uc    place                          = 0;
    uc    plural                         = 0;
    uc   *data                           = NULL;
    uc    type                           = 0;
    ui    size                           = 0;
    usi   word                           = 0;

    opt                                  = getopt (argc, argv, "1ab:hlsV");
    while (opt                          != -1)
    {
        switch (opt)
        {
            case 'a':
				header                   = 4;
                break;

            case '1':
                flags                    = flags | FLAG_VERTICAL;
                break;

            case 'b':
                if ((flags & FLAG_BANK) != FLAG_BANK)
                {
                    banknum              = 0x00;
                }
                flags                    = flags | FLAG_BANK;
                byte                     = (uc) atoi (optarg);
                place                    = 1;
                place                    = place << byte;
                banknum                  = banknum | place;
                break;

            case 'h':
                display_help (argv[0]);
                break;

            case 'l':
                flags                    = flags | FLAG_LONG;
                space                    = 8;
                break;

            case 's':
                flags                    = flags | FLAG_SUMMARY;
                break;

            case 'V':
                display_version();
                break;

            default: /* '?' */
                fprintf (stderr, "Usage: %s [-b #] [-1hlV] name\n", argv[0]);
                exit (EXIT_FAILURE);
        }
        opt                              = getopt (argc, argv, "1ab:hlsV");
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // Obtain current terminal size
    //
    struct winsize terminal;
    ioctl (0, TIOCGWINSZ, &terminal);

    plural                               = (argc - optind);
    data                                 = (uc *) malloc (sizeof (uc) * 64);

    //////////////////////////////////////////////////////////////////////////
    //
    // Process for each file specified on command-line
    //
    for (index = optind; index < argc; index++)
    {
        in                               = fopen (argv[index], "r");
        if (in                          == NULL)
        {
            fprintf (stderr, "ERROR opening '%s' for reading\n", argv[index]);
            exit (1);
        }

        //////////////////////////////////////////////////////////////////////
        //
        // Read the bank/type byte
        //
        type                             = fgetc (in);
        offset                           = 0;
        if (((flags & FLAG_LONG)        == FLAG_LONG) ||
            (plural                     >  1))
        {
            if (plural                  >  1)
            {
                fprintf (stdout, "%s:\n", argv[index]);
            }
        }

        sprintf (data, "\0");
        while (!feof (in))
        {
            if ((flags & FLAG_LONG)     == FLAG_LONG)
            {
                sprintf (data, "0x%.6x: ", offset);
            }

            //////////////////////////////////////////////////////////////////
            //
            // Separate the bank/type byte
            //
            offset                       = offset + 1; // count the byte
            bank                         = type & 0xE0;
            bank                         = bank >> 5;

            //////////////////////////////////////////////////////////////////
            //
            // Isolate type from bank
            //
            type                         = type & 0x1F;
                
            //////////////////////////////////////////////////////////////////
            //
            // Read and de-endiafy the size bytes
            //
            size                         = fgetc (in); offset++;
            byte                         = fgetc (in); offset++;
            word                         = byte;
            word                         = word << 8;
            size                         = size | word;
            byte                         = fgetc (in); offset++; // reserved

            //////////////////////////////////////////////////////////////////
            //
            // Format output based on options provided
            //
            if ((flags & FLAG_LONG)     == FLAG_LONG)
            {
                sprintf (data, "%sbank: %u, type: ", data, bank);
            }
            else
            {
                sprintf (data, "%s%u:", data, bank);
            }

            //////////////////////////////////////////////////////////////////
            //
            // Populate information based on CHUNK type
            //
            if (type == CHUNK_DEFAULT)
            {
                sprintf (data, "%s%*s", data, space, "DEFAULT");
            }
            else if (type == CHUNK_TILES)
            {
                sprintf (data, "%s%*s", data, space, "TILES");
            }
            else if (type == CHUNK_SPRITES)
            {
                sprintf (data, "%s%*s", data, space, "SPRITES");
            }
            else if (type == CHUNK_COVER_DEP)
            {
                sprintf (data, "%s%*s", data, space, "COVER");
            }
            else if (type == CHUNK_MAP)
            {
                sprintf (data, "%s%*s", data, space, "MAP");
            }
            else if (type == CHUNK_CODE)
            {
                sprintf (data, "%s%*s", data, space, "CODE");
            }
            else if (type == CHUNK_FLAGS)
            {
                sprintf (data, "%s%*s", data, space, "FLAGS");
            }
            else if (type == CHUNK_SAMPLES)
            {
                sprintf (data, "%s%*s", data, space, "SAMPLES");
            }
            else if (type == CHUNK_WAVEFORM)
            {
                sprintf (data, "%s%*s", data, space, "WAVEFORM");
            }
            else if (type == CHUNK_PALETTE)
            {
                sprintf (data, "%s%*s", data, space, "PALETTE");
            }
            else if (type == CHUNK_PATTERNS_DEP)
            {
                sprintf (data, "%s%*s", data, space, "PATT_DEP");
            }
            else if (type == CHUNK_MUSIC)
            {
                sprintf (data, "%s%*s", data, space, "MUSIC");
            }
            else if (type == CHUNK_PATTERNS)
            {
                sprintf (data, "%s%*s", data, space, "PATTERNS");
            }
            else if (type == CHUNK_CODE_ZIP)
            {
                sprintf (data, "%s%*s", data, space, "CODE_ZIP");
            }
            else if (type == CHUNK_SCREEN)
            {
                sprintf (data, "%s%*s", data, space, "SCREEN");
            }
            else if (type == CHUNK_BINARY)
            {
                sprintf (data, "%s%*s", data, space, "BINARY");
            }
            else
            {
                sprintf (data, "%s%.*X", data, space, type);
            }

			if (((type                  == CHUNK_CODE)    ||
				 (type                  == CHUNK_BINARY)) &&
				(size                   == 0))
			{
				/*for (count = 0; count < 65536; count++)
				{
					byte                 = fgetc (in);
					size                 = size + 1;
				}
				size                     = size - 1;*/
				size                     = 65536;
			}

            if ((flags & FLAG_LONG)     == FLAG_LONG)
            {
                sprintf (data, "%s, size: 0x%.5x (%6u) bytes\n", data,
				                                                 (size + header),
																 (size + header));
            }

            //////////////////////////////////////////////////////////////////
            //
            // Filter for specified banks in banknum (all by default)
            //
            place                        = 1;
            place                       == place << bank;
            if ((banknum & place)       == place)
            {
                fprintf (stdout, "%s", data);
            }

            //////////////////////////////////////////////////////////////////
            //
            // Format for default listing behaviour
            //
            if (flags                   == FLAG_DEFAULT)
            {
                indent                   = indent + 1;
                if ((indent % 8)        == 0)
                {
                    fprintf (stdout, "\n");
                    indent               = 1;
                }
            }

            if ((flags & FLAG_VERTICAL) == FLAG_VERTICAL)
            {
                if ((flags & FLAG_LONG) != FLAG_LONG)
                {
                    fprintf (stdout, "\n");
                }
            }

			fseek (in, size, SEEK_CUR);
			offset                       = offset + size;

			//////////////////////////////////////////////////////////////////
			//
			// Read the bank/type byte
			//
			type                         = fgetc (in);

            sprintf (data, "\0");
        }
        fclose (in);

        if (((flags & FLAG_LONG)        != FLAG_LONG) &&
            ((flags & FLAG_VERTICAL)    != FLAG_VERTICAL))
        {
            fprintf (stdout, "\n");
        }

		if ((flags & FLAG_SUMMARY)      == FLAG_SUMMARY)
		{
			fprintf (stdout, "--------------------------------");
			fprintf (stdout, "-------------------------------\n");
			fprintf (stdout, "                                  ");
			fprintf (stdout, "total: 0x%.5x (%6u) bytes\n\n", offset, offset);
		}

        if ((plural                     >  1) &&
            ((argc - index)             >  1)) 
        {
            fprintf (stdout, "\n");
        }
    }

    return (0);
}

void    display_help(char *argv)
{
    fprintf (stdout, "Usage: %s [OPTION].. FILE..\n\n", argv);
    fprintf (stdout, " -1    vertical listing\n");
    fprintf (stdout, " -a    all bytes, including header\n");
    fprintf (stdout, " -b #  only show results for indicated bank\n");
    fprintf (stdout, " -h    show this help\n");
    fprintf (stdout, " -l    detailed information listing\n");
    fprintf (stdout, " -s    provide overall summary\n");
    fprintf (stdout, " -V    show version information\n");
    exit (0);
}

void    display_version()
{
    fprintf (stdout, "tictools ls (tls/ticls) release 20230423-1441\n");
    exit (0);
}
