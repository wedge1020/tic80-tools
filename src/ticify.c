#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_TILES         1
#define	CHUNK_SPRITES       2
#define	CHUNK_COVER_DEP	    3 // deprecated as of 0.90
#define	CHUNK_MAP           4
#define	CHUNK_CODE          5
#define	CHUNK_FLAGS         6
#define	CHUNK_SAMPLES       9
#define	CHUNK_WAVEFORM     10
#define	CHUNK_PALETTE      12
#define	CHUNK_PATTERNS_DEP 13 // deprecated as of 0.80
#define	CHUNK_MUSIC        14
#define	CHUNK_PATTERNS     15
#define	CHUNK_CODE_ZIP     16 // deprecated as of 1.00
#define	CHUNK_DEFAULT      17
#define	CHUNK_SCREEN       18
#define	CHUNK_BINARY       19
//7, 8, 11	(reserved) ...		
//19..31	(reserved) ...

typedef unsigned char      uc;
typedef unsigned short int usi;

int main(int argc, char **argv)
{
	FILE *in       = NULL;
	FILE *out      = NULL;
	int   counter  = 0;
	int   index    = 0;
	int   offset   = 0;
	uc    bank     = 0;
	uc    byte     = 0;
	uc    type     = 0;
	usi   size     = 0;
	usi   word     = 0;
	char *outfile  = NULL;

	for (index = 1; index < argc; index++)
	{
		in       = fopen (argv[index], "r");
		if (in  == NULL)
		{
			fprintf (stderr, "ERROR opening '%s' for reading\n", argv[index]);
			exit (1);
		}

		// stat the file, get the size, determine the type, output the TIC format

		//////////////////////////////////////////////////////////////////////
		//
		// Read the bank/type byte
		//
		type      = fgetc (in);
		offset    = 0;
		fprintf (stdout, "%s:\n", argv[index]);
		while (!feof (in))
		{
			outfile   = (char *) malloc (sizeof (char) * strlen (argv[index]) + 12);
			fprintf (stdout, "0x%.4x: ", offset);

			//////////////////////////////////////////////////////////////////
			//
			// Separate the bank/type byte
			//
			offset    = offset + 1; // count the byte
			bank      = type & 0xE0;
			bank      = bank >> 5;

			//////////////////////////////////////////////////////////////////
			//
			// Read and de-endiafy the size bytes
			//
			size      = fgetc (in); offset++;
			byte      = fgetc (in); offset++;
			word      = byte;
			word      = word << 8;
			size      = size | word;
			byte      = fgetc (in); offset++; // reserved

			if (type == CHUNK_DEFAULT)
			{
				fprintf (stdout, "type: DEFAULT, size: %.4hX, data: NONE\n", size);
				sprintf (outfile, "%s.default", argv[index]);
			}
			else if (type == CHUNK_SPRITES)
			{
				fprintf (stdout, "type: SPRITES, size: %.4hX, data: SAVE\n", size);
				sprintf (outfile, "%s.sprites", argv[index]);
			}
			else if (type == CHUNK_CODE)
			{
				fprintf (stdout, "type: CODE,    size: %.4hX, data: SAVE\n", size);
				sprintf (outfile, "%s.code", argv[index]);
			}
			else if (type == CHUNK_SAMPLES)
			{
				fprintf (stdout, "type: SAMPLES, size: %.4hX, data: SAVE\n", size);
				sprintf (outfile, "%s.samples", argv[index]);
			}
			else
			{
				fprintf (stdout, "type: %7.2hhX, size: %.4hX, data: SAVE\n", type, size);
				sprintf (outfile, "%s.%.2hhX", argv[index], type);
			}

			out            = fopen (outfile, "w");
			if (out       == NULL)
			{
					fprintf (stderr, "ERROR: could not open '%s' for writing\n", outfile);
					exit (2);
			}
			
			for (counter = 1; counter <= size; counter++)
			{
				byte       = fgetc (in);
				offset     = offset + 1;
				if (out   != NULL)
					fprintf (out, "%c", byte);
			}

			if (out       != NULL)
			{
				fclose (out);
				free (outfile);
				outfile    = NULL;
			}

			//////////////////////////////////////////////////////////////////
			//
			// Read the bank/type byte
			//
			type      = fgetc (in);
		}
		fclose (in);
		fprintf (stdout, "\n");
	}

	return (0);
}
