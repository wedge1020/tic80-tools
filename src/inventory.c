typedef unsigned char      ussi;
typedef unsigned short int usi;
typedef unsigned int       ui;

struct chunk {
	ui            offset;
	ussi          bank;
	ussi          type;
	usi           size;
	ussi          reserved;
	ussi         *data;
	struct chunk *next;
};
typedef struct chunk Chunk;

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
