#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char* file_arg = NULL;
char* output_dir = ".";
char* input_dir = ".";
int width = 4096 - 32*2, height = 2160 - 32*2;
int font_width = 16, font_height = 16;
int start_row = 0;

int decode();
int encode();

int main( int argc, char* argv[] )
{
    int arg_index = 1;
    int decode_mode = 0;
    
    while ( arg_index < argc )
    {
        if ( !strcmp( argv[arg_index], "-d" ) || !strcmp( argv[arg_index], "--decode" ))
        {
            decode_mode = 1;
        }
        else if ( !strcmp( argv[arg_index], "-o" ) || !strcmp( argv[arg_index], "--out-dir" ))
        {
            arg_index++;
            if ( (arg_index >= argc) || (argv[arg_index][0] == '-') )
            {
                fprintf( stderr, "-o requires a directory name\n" );
                return  -1;
            }
            output_dir = argv[arg_index];
        }
        else if ( !strcmp( argv[arg_index], "-i" ) || !strcmp( argv[arg_index], "--in-dir" ))
        {
            arg_index++;
            if ( (arg_index >= argc) || (argv[arg_index][0] == '-') )
            {
                fprintf( stderr, "-i requires a directory name\n" );
                return  -1;
            }
            input_dir = argv[arg_index];
        }
        else if ( !strcmp( argv[arg_index], "--font" ) )
        {
            arg_index++;
            if ( (arg_index >= argc) || (argv[arg_index][0] == '-') )
            {
                fprintf( stderr, "--font requires a WxH argument\n" );
                return  -1;
            }
            int count = sscanf( argv[arg_index], "%dx%d", &font_width, &font_height);
            if ( count != 2 )
            {
                fprintf( stderr, "--font requires a WxH argument\n" );
            }
        }
        else if ( !strcmp( argv[arg_index], "--start-row" ) )
        {
            arg_index++;
            if ( (arg_index >= argc) || (argv[arg_index][0] == '-') )
            {
                fprintf( stderr, "--start-row requires an argument\n" );
                return  -1;
            }
            int count = sscanf( argv[arg_index], "%d", &start_row);
            if ( count != 1 )
            {
                fprintf( stderr, "--start-row requires an argument\n" );
            }
        }
        else if ( !strcmp( argv[arg_index], "--width" ) )
        {
            arg_index++;
            if ( (arg_index >= argc) || (argv[arg_index][0] == '-') )
            {
                fprintf( stderr, "--width requires an argument\n" );
                return  -1;
            }
            int count = sscanf( argv[arg_index], "%d", &width);
            if ( count != 1 )
            {
                fprintf( stderr, "--width requires an argument\n" );
            }
        }
        else if ( !strcmp( argv[arg_index], "--height" ) )
        {
            arg_index++;
            if ( (arg_index >= argc) || (argv[arg_index][0] == '-') )
            {
                fprintf( stderr, "--height requires an argument\n" );
                return  -1;
            }
            int count = sscanf( argv[arg_index], "%d", &height);
            if ( count != 1 )
            {
                fprintf( stderr, "--height requires an argument\n" );
            }
        }
        else if ( !strcmp( argv[arg_index], "-h" ) || !strcmp( argv[arg_index], "--help" ) )
        {
            printf(
                "Usage: ivm-hex-dump (options) <file>\n"
                "\n"
                "  Encoding options:"
                "    -d|--decode       : Run in decode mode\n"
                "    -o|--out-dir      : Write encoded pages here\n"
                "    --width <widht>   : Width in pixels of target device\n"
                "    --height <height> : Height in pixels of target device\n"
                "    --font <WxH>      : Font width x height in pixels\n"
                "\n"
                "  Decoding options:\n"
                "    -i|--in-dir       : Read encoded pages from here\n"
                "\n"
                "  Common options:\n"
                "    --start-row       : Start row, can be used for partial coding / decoding\n"
                "\n"
                "In encode mode the application will hex encode the input file ascii text pages, where each page has "
                "width / font_width columns and height / font_height rows. A hex encoded byte uses two "
                "colums each containing one of [abcdef0123456789]. Each row starts with a 3 digit hex line number, space, "
                "then 3 digit hex checksum and ending with space.\n"
                "If height is 0, one page will be created.\n"
                "Pages are written to the output folder named: <out-dir>/%%05d.txt\n"
                "\n"
                "In decode mode the application will read text files in input folder: %%05d.txt"
                "and decode to binary <file>, reporting error on checksum mismatch.\n"
                );
            return  0;
        }
        else if (argv[arg_index][0] != '-')
        {
            file_arg = argv[arg_index];
        }
        else
        {
            fprintf( stderr, "Unknown command line argument: %s\n", argv[arg_index] );
            return 1;
        }
        
        arg_index++;
    }


    if ( decode_mode )
    {
        return decode();
    } 
    else
    {
        return encode();
    }
    
    return 0;
}

FILE* open_page( int page, char* dir, char* mode, int print_error )
{
    char file_name[256];
    sprintf( file_name, "%s/%05d.txt", dir, page );
    FILE* file = fopen( file_name, mode );

    if ( file == NULL )
    {
        if ( print_error )
        {
            fprintf( stderr, "Failed to open %s\n", file_name );
        }
    }
    else
    {
        if ( *mode == 'r' )
        {
            printf( "Reading %s\n", file_name );
        }
        else
        {
            printf( "Creating %s\n", file_name );
        }
    }
    
    return file;
}

unsigned int checksum(char* buffer, unsigned int size, unsigned int line)
{
    unsigned int c = 0;
    unsigned int i = 0;
    while ( i < size )
    {
        c += (256*line + i + 2) * buffer[i] + 1;
        i++;
    }

    return c % 4093;
}


int decode()
{
    FILE* out = fopen( file_arg, "w" );
    if ( out == NULL )
    {
        fprintf( stderr, "Failed to create file %s\n", file_arg );
        return 1;
    }
    printf( "Decoding...\n" );

    int page = 0;
    int row = start_row;
    FILE* file = open_page( page, input_dir, "r", 1 );
    while ( file )
    {
        int size = 2048;
        char buffer[size];

        while( fgets( buffer, size, file ) )
        {
            unsigned int line, check;
            char data[2048];
            int convert = sscanf( buffer, "%3x %3x %s", &line, &check, data );
            if ( convert != 3 )
            {
                fprintf( stderr, "Illegal line format: %s\n",buffer );
                return 1;
            }

            int data_length = strlen( data );
            if ( data_length % 2 != 0 )
            {
                fprintf( stderr, "Illegal hex digits in line: %s\n", buffer );
                return 1;
            }
            
            char bytes[2048];
            int bytes_length = 0;
            int read_pos = 0;
            while ( sscanf( &data[read_pos], "%2hhx", &bytes[bytes_length]) == 1 )
            {
                read_pos += 2;
                bytes_length++;
            }
            
            size_t write_size = fwrite( bytes, bytes_length, 1, out );
            if ( write_size != 1 )
            {
                fprintf( stderr, "Failed to write to file: %s\n", file_arg );
            }

            int c = checksum( bytes, bytes_length, row );
            if ( c != check )
            {
                fprintf( stderr, "Checksum mismatch in line: %s\n", buffer );
            }

            row++;
        }

        
        page++;
        file = open_page( page, input_dir, "r", 0 );
    }

    return 0;
}


int encode()
{
    FILE* file = fopen( file_arg, "r" );
    if ( file == NULL )
    {
        fprintf( stderr, "Failed to open file %s\n", file_arg );
        return 1;
    }
    printf( "Encoding...\n" );
    
    FILE* out = NULL;
    int page = -1;
    int page_width = width / font_width;
    int page_height = height / font_height;
    printf( "Encoding to %dx%d page size with font size: %dx%d\n",
            page_width, page_height, font_width, font_height );

    int checksum_width = 8;
    int data_width = (page_width - checksum_width) / 2;
    if ( data_width <= 0 )
    {
        fprintf( stderr, "Page width too small, need room room for data\n" );
    }
    
    char buffer[data_width];
    int buffer_size = fread( buffer, 1, data_width, file );
    unsigned int row = start_row; 
    while ( buffer_size )
    {
        unsigned int c = checksum(buffer, buffer_size, row);
        
        // New page?
        if ( row == start_row || ( page_height != 0 && row % page_height == 0 ) )
        {
            if ( out ) fclose( out );
            page++;
            out = open_page( page, output_dir, "w", 1 );
            if ( out == NULL )
            {
                return 1;
            }
        }

        // Checksum
        fprintf( out, "%.3x %.3x ", row, c );
        
        // Data
        int read = 0;
        while ( read < buffer_size )
        {
            fprintf( out, "%.2hhx", buffer[read] );
            read++;
        }
        fprintf( out, "\n" );

        buffer_size = fread( buffer, 1, sizeof(buffer), file );
        row++;
    }

    return 0;
}
