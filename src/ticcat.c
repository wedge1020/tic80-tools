#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <caca.h>
##define TB_IMPL
#include <termbox2.h>

#define  NUM_BANKS           8
#define  NUM_TYPES           20

#define  CHUNK_TILES         1
#define  CHUNK_SPRITES       2
#define  CHUNK_COVER_DEP     3  // deprecated as of 0.90
#define  CHUNK_MAP           4
#define  CHUNK_CODE          5
#define  CHUNK_FLAGS         6
#define  CHUNK_SAMPLES       9
#define  CHUNK_WAVEFORM      10
#define  CHUNK_PALETTE       12
#define  CHUNK_PATTERNS_DEP  13 // deprecated as of 0.80
#define  CHUNK_MUSIC         14
#define  CHUNK_PATTERNS      15
#define  CHUNK_CODE_ZIP      16 // deprecated as of 1.00
#define  CHUNK_DEFAULT       17
#define  CHUNK_SCREEN        18
#define  CHUNK_BINARY        19

#define  FLAG_DEFAULT        0
#define  FLAG_BANK           1
#define  FLAG_TYPE           2
#define  FLAG_CHUNK          4
#define  FLAG_BIN            8
#define  FLAG_HEX            16
#define  FLAG_HEADER         32

#define  TRUE                1
#define  FALSE               0

typedef struct   tb_event    TB_Event;
typedef struct   winsize     WinSize;
typedef unsigned char        uc;
typedef unsigned short int   usi;
typedef unsigned int         ui;

ui                           flags;
FILE                        *in;
TB_Event                     ev;
ui                           offset;
WinSize                      terminal;
uc                           verbosity;

uc      name2type (char *);
char   *type2name (uc);
void    display_help (char *);
void    display_version (void);
void    sprite_view (usi);

int main (int argc, char **argv)
{
    int   counter                           = 0;
    int   index                             = 0;
    int   opt                               = 0;            // getopt
    int   status                            = 0;
    uc    bank                              = 0;
    uc    byte                              = 0;
    uc    custom                            = FALSE;
    uc    type                              = 0;
    ui    bound                             = 0;
    ui    filesize                          = 0;
    usi   size                              = 0;
    usi   word                              = 0;
    usi   adjust                            = 0;
    uc    list[NUM_BANKS][NUM_TYPES];

    flags                                   = FLAG_DEFAULT;
    in                                      = NULL;
    offset                                  = 0;
    verbosity                               = FALSE;

    for (bank = 0; bank < NUM_BANKS; bank++)
    {
        for (type = 0; type < NUM_TYPES; type++)
        {
            list[bank][type]                = TRUE;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Process command-line arguments
    //
    opt                                     = getopt(argc, argv, "Bb:c:t:HhVvx");
    while (opt                             != -1)
    {
        switch (opt)
        {
            case 'B':
                flags                       = flags | FLAG_BIN;
                break;

            case 'H':
                flags                       = flags | FLAG_HEADER;
                break;

            case 'b':
                if (custom                 == FALSE)
                {
                    custom                  = TRUE;
                    for (bank = 0; bank < NUM_BANKS; bank++)
                    {
                        for (type = 0; type < NUM_TYPES; type++)
                        {
                            list[bank][type]= FALSE;
                        }
                    }
                }
                flags                       = flags | FLAG_BANK;
                bank                        = (uc) atoi (optarg);

                if (verbosity              == TRUE)
                    fprintf (stderr, "[verbose] setting bank %hhu\n", bank);

                for (index                  = 0;
                     index                 <  NUM_TYPES;
                     index                  = index + 1)
                {
                    list[bank][index]       = TRUE;
                }
                break;

            case 'c':
                if (custom                 == FALSE)
                {
                    custom                  = TRUE;
                    for (bank = 0; bank < NUM_BANKS; bank++)
                    {
                        for (type = 0; type < NUM_TYPES; type++)
                        {
                            list[bank][type]= FALSE;
                        }
                    }
                }
                flags                       = flags | FLAG_CHUNK;
                bank                        = optarg[0] - 48;
                type                        = name2type ((optarg+2));

                if (verbosity              == TRUE)
                {
                    fprintf (stderr, "setting bank %hhu, type: %s (%hhu)\n", bank,
                                                                             (optarg+2),
                                                                             type);
                }

                list[bank][type]            = TRUE;
                break;

            case 't':
                if (custom                 == FALSE)
                {
                    custom                  = TRUE;
                    for (bank = 0; bank < NUM_BANKS; bank++)
                    {
                        for (type = 0; type < NUM_TYPES; type++)
                        {
                            list[bank][type]= FALSE;
                        }
                    }
                }
                flags                       = flags | FLAG_CHUNK;
                type                        = name2type (optarg);

                if (verbosity              == TRUE)
                {
                    fprintf (stderr, "setting type %s (%hhu)\n", optarg, type);
                }

                for (index                  = 0;
                     index                 <  NUM_BANKS;
                     index                  = index + 1)
                {
                    list[index][type]       = TRUE;
                }
                break;

            case 'h':
                display_help (argv[0]);
                break;

            case 'V':
                display_version ();
                break;

            case 'v':
                verbosity                   = TRUE;
                break;

            case 'x':
                flags                       = flags | FLAG_HEX;
                break;

            default: /* '?' */
                fprintf (stderr, "Usage: %s [OPTION..] FILE.tic..\n", argv[0]);
                exit (EXIT_FAILURE);
        }
        opt                                 = getopt (argc, argv, "Bb:c:t:HhVvx");
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Obtain terminal dimensions
    //
    ioctl  (0, TIOCGWINSZ, &terminal);

/*
    for (bank = 0; bank < NUM_BANKS; bank++)
    {
        for (type = 0; type < NUM_TYPES; type++)
        {
            fprintf (stdout, "[%hhu][%hhu]: %hhu\n", bank, type, list[bank][type]);
        }
    }
*/
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Process remaining arguments (TIC files specified)
    //
    for (index = optind; index < argc; index++)
    {
        in                                  = fopen (argv[index], "r");
        if (in                             == NULL)
        {
            fprintf (stderr, "ERROR opening '%s' for reading\n", argv[index]);
            exit (1);
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Obtain the file size, in bytes
        //
        fseek (in, 0, SEEK_END);
        filesize                            = ftell (in);
        fseek (in, 0, SEEK_SET);

        ////////////////////////////////////////////////////////////////////////////////
        //
        // Read the bank/type byte
        //
        offset                              = 0;
        type                                = fgetc (in);

        if (verbosity                      == 1)
        {
            fprintf (stderr, "%s:\n", argv[index]);
        }

        while (!feof (in))
        {
            ////////////////////////////////////////////////////////////////////////////
            //
            // Separate the bank/type bytes
            //
            offset                          = offset + 1;  // count the byte
            bank                            = type & 0xE0; // isolate type data
            bank                            = bank >> 5;   // place into one's place
            type                            = type & 0x1F; // strip bank bits

            //fprintf (stdout, "type is: %hhu\n", type);

            ////////////////////////////////////////////////////////////////////////////
            //
            // Read and de-endiafy the size bytes
            //
            size                            = fgetc (in); offset++;
            byte                            = fgetc (in); offset++;
            word                            = byte;
            word                            = word << 8;
            size                            = size | word;
            byte                            = fgetc (in); offset++; // reserved

            ////////////////////////////////////////////////////////////////////////////
            //
            // Determine loop starting point and upper bound; if a large code chunk,
            // we go from the current position to the end of the file. Otherwise, we
            // go from 0 to the size of the particular chunk.
            //
            if (((type                     == CHUNK_CODE)    ||
				 (type                     == CHUNK_BINARY)) &&
                (size                      == 0))
            {
                bound                       = 65536;
                counter                     = offset;
            }
            else
            {
                bound                       = size;
                counter                     = 0;
            }

            if (list[bank][type]           == TRUE)
            {
                if ((flags & FLAG_HEADER)  == FLAG_HEADER)
                {
                    if (FLAG_BIN           == (flags & FLAG_BIN))
                    {
                        fprintf (stdout, "%c", (type | (bank << 5)));
                        fprintf (stdout, "%c", (size & 0xFF));
                        fprintf (stdout, "%c", (size >> 8));
                        fprintf (stdout, "%c", byte);
                    }
                    else //if (FLAG_HEX    == (flags & FLAG_HEX))
                    {
                        fprintf (stdout, "0x%.5X: bank: %hhu, ",     (offset-4),  bank);
                        fprintf (stdout, "type: %8s (%2hhu), ", type2name (type), type);
                        fprintf (stdout, "size: %u bytes\n",  (bound - counter));
                    }
                }

                if ((FLAG_BIN              != (flags & FLAG_BIN)) &&
                    (FLAG_HEX              != (flags & FLAG_HEX)) &&
                    (type                  != CHUNK_CODE))
                {
                    fprintf (stdout, "         ");
                }

                if ((flags & FLAG_HEX)     == FLAG_HEX)
                {
                    fprintf (stdout, "0x%.5X: ", offset);
                }

                while (counter             <  bound)
                {
                    byte                    = fgetc (in);
                    offset                  = offset  + 1;
                    if (FLAG_BIN           == (flags & FLAG_BIN))
                    {
                        fprintf (stdout, "%c",    byte);
                    }
                    else if (FLAG_HEX      == (flags & FLAG_HEX))
                    {
                        fprintf (stdout, "%.2X ", byte);
                        if (counter        == bound)
                        {
                            fprintf (stdout, "\n");
                        }
                        else if ((counter  >  0)               &&
                                 (0        == (counter % 16)))
                        {
                            fprintf (stdout, "\n0x%.5X: ", offset);
                        }
                    }
                    else
                    {
                        switch (type)
                        {
                            case CHUNK_DEFAULT:
                                break;

/*
                            case CHUNK_SPRITES:
                                adjust      = 256;
                                break;

                            case CHUNK_TILES:
                                adjust      = 0;

    status                     = tb_init ();
    if (status                == -1)
    {
        fprintf (stderr, "ERROR: could not activate terminal management\n");
        exit (1);
    }
                                break;
*/
                            case CHUNK_CODE:
                                fprintf (stdout, "%c",    byte);
                                break;

                            default:
                                if (bound  == counter - 1)
                                {
                                    fprintf (stdout, "\n");
                                }
                                else if ((counter  >  0)        &&
                                         (0 == (counter % 16)))
                                {
                                    fprintf (stdout, "\n         ");
                                }
                                fprintf (stdout, "%.2X ", byte);
                                break;
                        }
                    }
                    counter                 = counter + 1;
                }

                if ((flags & FLAG_BIN)     != FLAG_BIN)
                {
                    fprintf (stdout, "\n");
                    if ((bound             == counter) &&
                        (size              != 0))
                    {
                        fprintf (stdout, "\n");
                    }
                }
            }
            else
            {
                if (verbosity              == 1)
                {
                    fprintf (stdout, "[%.5X] bound: %u, counter: %u, size: %hu, filesize: %u\n", offset, bound, counter, size, filesize);
                    fprintf (stdout, "skipping %u bytes\n", (bound - counter));
                }

                fseek (in, (bound - counter), SEEK_CUR);
                offset                      = offset + (bound - counter);
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Read the bank/type byte
            //
            type                            = fgetc (in);
        }
        fclose (in);
        //fprintf (stdout, "\n");
    }

//    tb_printf (4, 8, 0, 0, "press any key to quit...");
//    tb_present ();
//
//    tb_poll_event (&ev);
//    tb_shutdown ();

    return (0);
}

uc  name2type (char *name)
{
    int  result  = 0;
    uc   type    = 0;

    result       = strncasecmp (name, "list",     4);
    if (result  == 0)
    {
        fprintf (stdout, "Available chunk types are:\n");
        fprintf (stdout, "%12s%12s%12s%12s\n", "tiles", "sprites",  "map",      "code");
        fprintf (stdout, "%12s%12s%12s%12s\n", "flags", "samples",  "waveform", "palette");
        fprintf (stdout, "%12s%12s%12s%12s\n", "music", "patterns", "default",  "screen");
        fprintf (stdout, "%12s\n",             "binary");
        exit (0);
    }

    result       = strncasecmp (name, "tiles",    5);
    if (result  == 0)
        type     = CHUNK_TILES;

    result       = strncasecmp (name, "sprites",  7);
    if (result  == 0)
        type     = CHUNK_SPRITES;

    result       = strncasecmp (name, "cover",    5);
    if (result  == 0)
        type     = CHUNK_COVER_DEP;

    result       = strncasecmp (name, "map",      3);
    if (result  == 0)
        type     = CHUNK_MAP;

    result       = strncasecmp (name, "code",     4);
    if (result  == 0)
        type     = CHUNK_CODE;

    result       = strncasecmp (name, "flags",    5);
    if (result  == 0)
        type     = CHUNK_FLAGS;

    result       = strncasecmp (name, "samples",  7);
    if (result  == 0)
        type     = CHUNK_SAMPLES;

    result       = strncasecmp (name, "waveform", 8);
    if (result  == 0)
        type     = CHUNK_WAVEFORM;

    result       = strncasecmp (name, "palette",  7);
    if (result  == 0)
        type     = CHUNK_PALETTE;

    result       = strncasecmp (name, "patt_dep", 8);
    if (result  == 0)
        type     = CHUNK_PATTERNS_DEP;

    result       = strncasecmp (name, "music",    5);
    if (result  == 0)
        type     = CHUNK_MUSIC;

    result       = strncasecmp (name, "patterns", 8);
    if (result  == 0)
        type     = CHUNK_PATTERNS;

    result       = strncasecmp (name, "code_zip", 8);
    if (result  == 0)
        type     = CHUNK_CODE_ZIP;

    result       = strncasecmp (name, "default",  7);
    if (result  == 0)
        type     = CHUNK_DEFAULT;

    result       = strncasecmp (name, "screen",   6);
    if (result  == 0)
        type     = CHUNK_SCREEN;

    result       = strncasecmp (name, "binary",   6);
    if (result  == 0)
        type     = CHUNK_BINARY;

    return (type);
}

char *type2name (uc type)
{
    char name[9];

    if (type       == CHUNK_TILES)
        sprintf (name, "tiles");
    else if (type  == CHUNK_SPRITES)
        sprintf (name, "sprites");
    else if (type  == CHUNK_COVER_DEP)
        sprintf (name, "cover");
    else if (type  == CHUNK_MAP)
        sprintf (name, "map");
    else if (type  == CHUNK_CODE)
        sprintf (name, "code");
    else if (type  == CHUNK_FLAGS)
        sprintf (name, "flags");
    else if (type  == CHUNK_SAMPLES)
        sprintf (name, "samples");
    else if (type  == CHUNK_WAVEFORM)
        sprintf (name, "waveform");
    else if (type  == CHUNK_PALETTE)
        sprintf (name, "palette");
    else if (type  == CHUNK_PATTERNS_DEP)
        sprintf (name, "patt_dep");
    else if (type  == CHUNK_MUSIC)
        sprintf (name, "music");
    else if (type  == CHUNK_PATTERNS)
        sprintf (name, "patterns");
    else if (type  == CHUNK_DEFAULT)
        sprintf (name, "default");
    else if (type  == CHUNK_CODE_ZIP)
        sprintf (name, "code_zip");
    else if (type  == CHUNK_SCREEN)
        sprintf (name, "screen");
    else if (type  == CHUNK_BINARY)
        sprintf (name, "binary");
    else
        sprintf (name, "other");

    return (name);
}

void    sprite_view (usi size)
{
    caca_canvas_t  *cv                = NULL;
    caca_display_t *dp                = NULL;
    int             bytecnt           = 0;
    int             counter           = 0;
    int             num               = 0;
    int             row               = 8;
    int             col               = 4;
    uc              block[256][row][col];
    uc              byte              = 0;
    uc             *color             = NULL;
    uc             *data              = NULL;
    usi             adjust            = 0;
    usi             x                 = 0;
    usi             y                 = 0;

    cv                                = caca_create_canvas (terminal.ws_col,
                                                            terminal.ws_row);

    color                             = (uc *) malloc (sizeof (uc) * 16);
    data                              = (uc *) malloc (sizeof (uc) * 32);

    if(cv                            == NULL)
    {
        fprintf (stderr, "ERROR: Failed to initialize libcaca (canvas)\n");
        exit (1);
    }

    dp                                = caca_create_display (cv);
    if(dp                            == NULL)
    {
        fprintf (stderr, "ERROR: Failed to initialize libcaca (display)\n");
        exit (2);
    }

    caca_set_color_ansi (cv, CACA_LIGHTGRAY, CACA_BLACK);
    caca_clear_canvas (cv);

    color[0]                          = CACA_BLACK;
    color[1]                          = CACA_MAGENTA;
    color[2]                          = CACA_RED;
    color[3]                          = CACA_BROWN;
    color[4]                          = CACA_YELLOW;
    color[5]                          = CACA_LIGHTGREEN;
    color[6]                          = CACA_LIGHTCYAN;
    color[7]                          = CACA_GREEN;
    color[8]                          = CACA_LIGHTMAGENTA;
    color[9]                          = CACA_LIGHTBLUE;
    color[10]                         = CACA_BLUE;
    color[11]                         = CACA_CYAN;
    color[12]                         = CACA_WHITE;
    color[13]                         = CACA_LIGHTRED;
    color[14]                         = CACA_LIGHTGRAY;
    color[15]                         = CACA_DARKGRAY;

    for (num = 0; num <= 255; num++)
    {
        for (row = 0; row < 8; row++)
        {
            for (col = 0; col < 4; col++)
            {
                if (bytecnt          <  size)
                {
                    byte              = fgetc (in);
                    offset            = offset + 1;
                    bytecnt           = bytecnt + 1;
                }
                else
                {
                    byte              = 0x00;
                }
                block[num][row][col]  = byte;
            }
        }
    }

    y = 0;
    for (num = 0; num <= 31;)
    //for (num = 0; num <= 255;)
    {
        x                             = 0;

        for (row = 0; row < 8; row++)
        {
            x                         = 0;
            if (row                  == 0)
            {
                for (counter = num; counter < (num + 8); counter++)
                {
                    //fprintf (stdout, "#%-3u       ", counter + adjust);
                    sprintf (data, "#%-3u", (counter + adjust));
                    caca_set_color_ansi (cv, CACA_LIGHTGRAY, CACA_BLACK);
                    caca_put_str (cv, x, y, data);
                    x                 = x + 11;
                }
                //fprintf (stdout, "\n");
                y                     = y + 1;
                x                     = 0;
            }

            for (counter = num; counter < (num + 8); counter++)
            {
                for (col = 0; col < 4; col++)
                {
                    byte              = block[counter][row][col];
                    byte              = byte & 0x0F;
                    caca_set_color_ansi (cv, 0, color[byte]);
                    caca_put_str (cv, x, y, " ");
                    x                 = x + 1;
                    byte              = block[counter][row][col];
                    byte              = byte & 0xF0;
                    byte              = byte >> 4;
                    caca_set_color_ansi (cv, 0, color[byte]);
                    caca_put_str (cv, x, y, " ");
                    x                 = x + 1;
                    //fprintf (stdout, "%X", (byte & 0x0F));
                    //fprintf (stdout, "%X", (byte & 0xF0) >> 4);
                }
                caca_set_color_ansi (cv, CACA_LIGHTGRAY, CACA_BLACK);
            //    fprintf (stdout, "   ");
                x                     = x + 3;
            }
            //fprintf (stdout, "\n");
            y                         = y + 1;
            x                         = 0;
        }
        num                           = num + 8;
        //fprintf (stdout, "\n");
        y                             = y + 1;
        x                             = 0;
        caca_refresh_display (dp);
        if (y                        == terminal.ws_row)
        {
            caca_refresh_display (dp);
            caca_get_event (dp, CACA_EVENT_KEY_PRESS, NULL, -1);
            caca_clear_canvas (cv);
            x                         = 0;
            //y                         = 0;
        }
    }

    caca_free_display (dp);
    caca_free_canvas (cv);

    free (color);
    free (data);

    color                             = NULL;
    data                              = NULL;
}

void    display_help (char *argv)
{
    fprintf (stdout, "Usage: %s [OPTION].. FILE.tic..\n\n", argv);
    fprintf (stdout, " -B         direct binary output, no text interpretation\n");
    fprintf (stdout, " -b #       indicate bank to access for transaction (0-8)\n");
    fprintf (stdout, " -c #:type  indicate chunk to access for transaction\n");
    fprintf (stdout, " -t type    indicate type to access for transaction\n");
    fprintf (stdout, " -H         include header information (in context)\n");
    fprintf (stdout, " -h         show this help\n");
    fprintf (stdout, " -V         show version information\n");
    fprintf (stdout, " -v         increase verbosity\n");
    fprintf (stdout, " -x         display all content in hexadecimal\n\n");
    fprintf (stdout, "note: for standalone bank or type specification, ALL of the\n");
    fprintf (stdout, "      complementing elements will be selected (ie if bank 1\n");
    fprintf (stdout, "      is specifed, ALL types are selected in bank 1, and if\n");
    fprintf (stdout, "      instead sprites are specified, all sprite sections in\n");
    fprintf (stdout, "      ALL banks are selected).  To be more specific to just\n");
    fprintf (stdout, "      ONE bank and ONE type, use the '-c' argument instead.\n");
    fprintf (stdout, "      Specifying 'list' as the type will list the  possible\n");
    fprintf (stdout, "      types.\n\n");
    exit (0);
}

void    display_version (void)
{
    fprintf (stdout, "tictools cat (tcat/ticcat) release 20230424-1811\n");
    exit (0);
}
