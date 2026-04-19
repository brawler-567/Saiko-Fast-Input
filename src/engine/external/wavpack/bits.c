 
 
 
 
 
 
 

 

 
 
 
 
 

#include "wavpack.h"

#include <string.h>
#include <ctype.h>

 

 

static void bs_read (Bitstream *bs);

void bs_open_read (Bitstream *bs, uchar *buffer_start, uchar *buffer_end, read_stream file, uint32_t file_bytes)
{
    CLEAR (*bs);
    bs->buf = buffer_start;
    bs->end = buffer_end;

    if (file) {
        bs->ptr = bs->end - 1;
        bs->file_bytes = file_bytes;
        bs->file = file;
    }
    else
        bs->ptr = bs->buf - 1;

    bs->wrap = bs_read;
}

 
 
 
 

static void bs_read (Bitstream *bs)
{
    if (bs->file && bs->file_bytes) {
        uint32_t bytes_read, bytes_to_read = bs->end - bs->buf;

        if (bytes_to_read > bs->file_bytes)
            bytes_to_read = bs->file_bytes;

        bytes_read = bs->file (bs->buf, bytes_to_read);

        if (bytes_read) {
            bs->end = bs->buf + bytes_read;
            bs->file_bytes -= bytes_read;
        }
        else {
            memset (bs->buf, -1, bs->end - bs->buf);
            bs->error = 1;
        }
    }
    else
        bs->error = 1;

    if (bs->error)
        memset (bs->buf, -1, bs->end - bs->buf);

    bs->ptr = bs->buf;
}

 

void little_endian_to_native (void *data, char *format)
{
    uchar *cp = (uchar *) data;
    int32_t temp;

    while (*format) {
        switch (*format) {
            case 'L':
                temp = cp [0] + ((int32_t) cp [1] << 8) + ((int32_t) cp [2] << 16) + ((int32_t) cp [3] << 24);
                * (int32_t *) cp = temp;
                cp += 4;
                break;

            case 'S':
                temp = cp [0] + (cp [1] << 8);
                * (short *) cp = (short) temp;
                cp += 2;
                break;

            default:
                if (isdigit (*format))
                    cp += *format - '0';

                break;
        }

        format++;
    }
}

void native_to_little_endian (void *data, char *format)
{
    uchar *cp = (uchar *) data;
    int32_t temp;

    while (*format) {
        switch (*format) {
            case 'L':
                temp = * (int32_t *) cp;
                *cp++ = (uchar) temp;
                *cp++ = (uchar) (temp >> 8);
                *cp++ = (uchar) (temp >> 16);
                *cp++ = (uchar) (temp >> 24);
                break;

            case 'S':
                temp = * (short *) cp;
                *cp++ = (uchar) temp;
                *cp++ = (uchar) (temp >> 8);
                break;

            default:
                if (isdigit (*format))
                    cp += *format - '0';

                break;
        }

        format++;
    }
}
