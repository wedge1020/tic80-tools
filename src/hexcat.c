#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uc;

int main(int argc, char **argv)
{
	FILE *in    = NULL;
	uc    byte  = 0;
	
	in          = fopen (argv[1], "r");
	if (in     == NULL)
	{
		fprintf (stderr, "ERROR opening '%s' for reading\n", argv[1]);
		exit (1);
	}

	byte        = fgetc (in);
	while (!feof(in))
	{
		fprintf (stdout, "%.2hhX ", byte);
		byte    = fgetc (in);
	}

	fclose (in);

	return (0);
}
