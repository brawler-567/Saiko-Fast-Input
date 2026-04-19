 
 
 
 
 
 
 

 

 
 
 
 
 
 
 
 

 
 
 
 
 
 

#include "wavpack.h"

#include <string.h>

 

#define LIMIT_ONES 16    

 
 
#define SLS 8
#define SLO ((1 << (SLS - 1)))

 
#define DIV0 128         
#define DIV1 64          
#define DIV2 32          

 
#define GET_MED(med) (((c->median [med]) >> 4) + 1)

 
 
 
 

#define INC_MED0() (c->median [0] += ((c->median [0] + DIV0) / DIV0) * 5)
#define DEC_MED0() (c->median [0] -= ((c->median [0] + (DIV0-2)) / DIV0) * 2)
#define INC_MED1() (c->median [1] += ((c->median [1] + DIV1) / DIV1) * 5)
#define DEC_MED1() (c->median [1] -= ((c->median [1] + (DIV1-2)) / DIV1) * 2)
#define INC_MED2() (c->median [2] += ((c->median [2] + DIV2) / DIV2) * 5)
#define DEC_MED2() (c->median [2] -= ((c->median [2] + (DIV2-2)) / DIV2) * 2)

#define count_bits(av) ( \
 (av) < (1 << 8) ? nbits_table [av] : \
  ( \
   (av) < (1L << 16) ? nbits_table [(av) >> 8] + 8 : \
   ((av) < (1L << 24) ? nbits_table [(av) >> 16] + 16 : nbits_table [(av) >> 24] + 24) \
  ) \
)

 

const char nbits_table [] = {
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,      
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,      
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,      
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,      
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,      
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,      
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,      
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,      
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8       
};

static const uchar log2_table [] = {
    0x00, 0x01, 0x03, 0x04, 0x06, 0x07, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x10, 0x11, 0x12, 0x14, 0x15,
    0x16, 0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x1e, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2a,
    0x2c, 0x2d, 0x2e, 0x2f, 0x31, 0x32, 0x33, 0x34, 0x36, 0x37, 0x38, 0x39, 0x3b, 0x3c, 0x3d, 0x3e,
    0x3f, 0x41, 0x42, 0x43, 0x44, 0x45, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4d, 0x4e, 0x4f, 0x50, 0x51,
    0x52, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63,
    0x64, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x74, 0x75,
    0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85,
    0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95,
    0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4,
    0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb2,
    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc0,
    0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcb, 0xcc, 0xcd, 0xce,
    0xcf, 0xd0, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd8, 0xd9, 0xda, 0xdb,
    0xdc, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe4, 0xe5, 0xe6, 0xe7, 0xe7,
    0xe8, 0xe9, 0xea, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xee, 0xef, 0xf0, 0xf1, 0xf1, 0xf2, 0xf3, 0xf4,
    0xf4, 0xf5, 0xf6, 0xf7, 0xf7, 0xf8, 0xf9, 0xf9, 0xfa, 0xfb, 0xfc, 0xfc, 0xfd, 0xfe, 0xff, 0xff
};

static const uchar exp2_table [] = {
    0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b,
    0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x12, 0x13, 0x13, 0x14, 0x15, 0x16, 0x16,
    0x17, 0x18, 0x19, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1d, 0x1e, 0x1f, 0x20, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x24, 0x25, 0x26, 0x27, 0x28, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3a, 0x3b, 0x3c, 0x3d,
    0x3e, 0x3f, 0x40, 0x41, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x48, 0x49, 0x4a, 0x4b,
    0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,
    0x5b, 0x5c, 0x5d, 0x5e, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x87, 0x88, 0x89, 0x8a,
    0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
    0x9c, 0x9d, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad,
    0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0,
    0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc8, 0xc9, 0xca, 0xcb, 0xcd, 0xce, 0xcf, 0xd0, 0xd2, 0xd3, 0xd4,
    0xd6, 0xd7, 0xd8, 0xd9, 0xdb, 0xdc, 0xdd, 0xde, 0xe0, 0xe1, 0xe2, 0xe4, 0xe5, 0xe6, 0xe8, 0xe9,
    0xea, 0xec, 0xed, 0xee, 0xf0, 0xf1, 0xf2, 0xf4, 0xf5, 0xf6, 0xf8, 0xf9, 0xfa, 0xfc, 0xfd, 0xff
};

static const char ones_count_table [] = {
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,7,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,8
};

 

void init_words (WavpackStream *wps)
{
    CLEAR (wps->w);
}

static int mylog2 (uint32_t avalue);

 
 
 

int read_entropy_vars (WavpackStream *wps, WavpackMetadata *wpmd)
{
    uchar *byteptr = wpmd->data;

    if (wpmd->byte_length != ((wps->wphdr.flags & MONO_DATA) ? 6 : 12))
        return FALSE;

    wps->w.c [0].median [0] = exp2s (byteptr [0] + (byteptr [1] << 8));
    wps->w.c [0].median [1] = exp2s (byteptr [2] + (byteptr [3] << 8));
    wps->w.c [0].median [2] = exp2s (byteptr [4] + (byteptr [5] << 8));

    if (!(wps->wphdr.flags & MONO_DATA)) {
        wps->w.c [1].median [0] = exp2s (byteptr [6] + (byteptr [7] << 8));
        wps->w.c [1].median [1] = exp2s (byteptr [8] + (byteptr [9] << 8));
        wps->w.c [1].median [2] = exp2s (byteptr [10] + (byteptr [11] << 8));
    }

    return TRUE;
}

 
 
 
 

int read_hybrid_profile (WavpackStream *wps, WavpackMetadata *wpmd)
{
    uchar *byteptr = wpmd->data;
    uchar *endptr = byteptr + wpmd->byte_length;

    if (wps->wphdr.flags & HYBRID_BITRATE) {
        wps->w.c [0].slow_level = exp2s (byteptr [0] + (byteptr [1] << 8));
        byteptr += 2;

        if (!(wps->wphdr.flags & MONO_DATA)) {
            wps->w.c [1].slow_level = exp2s (byteptr [0] + (byteptr [1] << 8));
            byteptr += 2;
        }
    }

    wps->w.bitrate_acc [0] = (int32_t)(byteptr [0] + (byteptr [1] << 8)) << 16;
    byteptr += 2;

    if (!(wps->wphdr.flags & MONO_DATA)) {
        wps->w.bitrate_acc [1] = (int32_t)(byteptr [0] + (byteptr [1] << 8)) << 16;
        byteptr += 2;
    }

    if (byteptr < endptr) {
        wps->w.bitrate_delta [0] = exp2s ((short)(byteptr [0] + (byteptr [1] << 8)));
        byteptr += 2;

        if (!(wps->wphdr.flags & MONO_DATA)) {
            wps->w.bitrate_delta [1] = exp2s ((short)(byteptr [0] + (byteptr [1] << 8)));
            byteptr += 2;
        }

        if (byteptr < endptr)
            return FALSE;
    }
    else
        wps->w.bitrate_delta [0] = wps->w.bitrate_delta [1] = 0;

    return TRUE;
}

 
 
 
 
 

void update_error_limit (struct words_data *w, uint32_t flags)
{
    int bitrate_0 = (w->bitrate_acc [0] += w->bitrate_delta [0]) >> 16;

    if (flags & MONO_DATA) {
        if (flags & HYBRID_BITRATE) {
            int slow_log_0 = (w->c [0].slow_level + SLO) >> SLS;

            if (slow_log_0 - bitrate_0 > -0x100)
                w->c [0].error_limit = exp2s (slow_log_0 - bitrate_0 + 0x100);
            else
                w->c [0].error_limit = 0;
        }
        else
            w->c [0].error_limit = exp2s (bitrate_0);
    }
    else {
        int bitrate_1 = (w->bitrate_acc [1] += w->bitrate_delta [1]) >> 16;

        if (flags & HYBRID_BITRATE) {
            int slow_log_0 = (w->c [0].slow_level + SLO) >> SLS;
            int slow_log_1 = (w->c [1].slow_level + SLO) >> SLS;

            if (flags & HYBRID_BALANCE) {
                int balance = (slow_log_1 - slow_log_0 + bitrate_1 + 1) >> 1;

                if (balance > bitrate_0) {
                    bitrate_1 = bitrate_0 * 2;
                    bitrate_0 = 0;
                }
                else if (-balance > bitrate_0) {
                    bitrate_0 = bitrate_0 * 2;
                    bitrate_1 = 0;
                }
                else {
                    bitrate_1 = bitrate_0 + balance;
                    bitrate_0 = bitrate_0 - balance;
                }
            }

            if (slow_log_0 - bitrate_0 > -0x100)
                w->c [0].error_limit = exp2s (slow_log_0 - bitrate_0 + 0x100);
            else
                w->c [0].error_limit = 0;

            if (slow_log_1 - bitrate_1 > -0x100)
                w->c [1].error_limit = exp2s (slow_log_1 - bitrate_1 + 0x100);
            else
                w->c [1].error_limit = 0;
        }
        else {
            w->c [0].error_limit = exp2s (bitrate_0);
            w->c [1].error_limit = exp2s (bitrate_1);
        }
    }
}

static uint32_t read_code (Bitstream *bs, uint32_t maxcode);

 
 
 
 
 
 
 

int32_t get_words (int32_t *buffer, int nsamples, uint32_t flags,
                struct words_data *w, Bitstream *bs)
{
    register struct entropy_data *c = w->c;
    int csamples;

    if (!(flags & MONO_DATA))
        nsamples *= 2;

    for (csamples = 0; csamples < nsamples; ++csamples) {
        uint32_t ones_count, low, mid, high;

        if (!(flags & MONO_DATA))
            c = w->c + (csamples & 1);

        if (!(w->c [0].median [0] & ~1) && !w->holding_zero && !w->holding_one && !(w->c [1].median [0] & ~1)) {
            uint32_t mask;
            int cbits;

            if (w->zeros_acc) {
                if (--w->zeros_acc) {
                    c->slow_level -= (c->slow_level + SLO) >> SLS;
                    *buffer++ = 0;
                    continue;
                }
            }
            else {
                for (cbits = 0; cbits < 33 && getbit (bs); ++cbits);

                if (cbits == 33)
                    break;

                if (cbits < 2)
                    w->zeros_acc = cbits;
                else {
                    for (mask = 1, w->zeros_acc = 0; --cbits; mask <<= 1)
                        if (getbit (bs))
                            w->zeros_acc |= mask;

                    w->zeros_acc |= mask;
                }

                if (w->zeros_acc) {
                    c->slow_level -= (c->slow_level + SLO) >> SLS;
                    CLEAR (w->c [0].median);
                    CLEAR (w->c [1].median);
                    *buffer++ = 0;
                    continue;
                }
            }
        }

        if (w->holding_zero)
            ones_count = w->holding_zero = 0;
        else {
            int next8;

            if (bs->bc < 8) {
                if (++(bs->ptr) == bs->end)
                    bs->wrap (bs);

                next8 = (bs->sr |= *(bs->ptr) << bs->bc) & 0xff;
                bs->bc += 8;
            }
            else
                next8 = bs->sr & 0xff;

            if (next8 == 0xff) {
                bs->bc -= 8;
                bs->sr >>= 8;

                for (ones_count = 8; ones_count < (LIMIT_ONES + 1) && getbit (bs); ++ones_count);

                if (ones_count == (LIMIT_ONES + 1))
                    break;

                if (ones_count == LIMIT_ONES) {
                    uint32_t mask;
                    int cbits;

                    for (cbits = 0; cbits < 33 && getbit (bs); ++cbits);

                    if (cbits == 33)
                        break;

                    if (cbits < 2)
                        ones_count = cbits;
                    else {
                        for (mask = 1, ones_count = 0; --cbits; mask <<= 1)
                            if (getbit (bs))
                                ones_count |= mask;

                        ones_count |= mask;
                    }

                    ones_count += LIMIT_ONES;
                }
            }
            else {
                bs->bc -= (ones_count = ones_count_table [next8]) + 1;
                bs->sr >>= ones_count + 1;
            }

            if (w->holding_one) {
                w->holding_one = ones_count & 1;
                ones_count = (ones_count >> 1) + 1;
            }
            else {
                w->holding_one = ones_count & 1;
                ones_count >>= 1;
            }

            w->holding_zero = ~w->holding_one & 1;
        }

        if ((flags & HYBRID_FLAG) && ((flags & MONO_DATA) || !(csamples & 1)))
            update_error_limit (w, flags);

        if (ones_count == 0) {
            low = 0;
            high = GET_MED (0) - 1;
            DEC_MED0 ();
        }
        else {
            low = GET_MED (0);
            INC_MED0 ();

            if (ones_count == 1) {
                high = low + GET_MED (1) - 1;
                DEC_MED1 ();
            }
            else {
                low += GET_MED (1);
                INC_MED1 ();

                if (ones_count == 2) {
                    high = low + GET_MED (2) - 1;
                    DEC_MED2 ();
                }
                else {
                    low += (ones_count - 2) * GET_MED (2);
                    high = low + GET_MED (2) - 1;
                    INC_MED2 ();
                }
            }
        }

        mid = (high + low + 1) >> 1;

        if (!c->error_limit)
            mid = read_code (bs, high - low) + low;
        else while (high - low > c->error_limit) {
            if (getbit (bs))
                mid = (high + (low = mid) + 1) >> 1;
            else
                mid = ((high = mid - 1) + low + 1) >> 1;
        }

        *buffer++ = getbit (bs) ? ~mid : mid;

        if (flags & HYBRID_BITRATE)
            c->slow_level = c->slow_level - ((c->slow_level + SLO) >> SLS) + mylog2 (mid);
    }

    return (flags & MONO_DATA) ? csamples : (csamples / 2);
}

 
 
 
 
 

static uint32_t read_code (Bitstream *bs, uint32_t maxcode)
{
    int bitcount = count_bits (maxcode);
    uint32_t extras = (1L << bitcount) - maxcode - 1, code;

    if (!bitcount)
        return 0;

    getbits (&code, bitcount - 1, bs);
    code &= (1L << (bitcount - 1)) - 1;

    if (code >= extras) {
        code = (code << 1) - extras;

        if (getbit (bs))
            ++code;
    }

    return code;
}

 
 
 
 
 

 
 
 
 


 
 

static int mylog2 (uint32_t avalue)
{
    int dbits;

    if ((avalue += avalue >> 9) < (1 << 8)) {
        dbits = nbits_table [avalue];
        return (dbits << 8) + log2_table [(avalue << (9 - dbits)) & 0xff];
    }
    else {
        if (avalue < (1L << 16))
            dbits = nbits_table [avalue >> 8] + 8;
        else if (avalue < (1L << 24))
            dbits = nbits_table [avalue >> 16] + 16;
        else
            dbits = nbits_table [avalue >> 24] + 24;

        return (dbits << 8) + log2_table [(avalue >> (dbits - 9)) & 0xff];
    }
}

 
 
 

int log2s (int32_t value)
{
    return (value < 0) ? -mylog2 (-value) : mylog2 (value);
}

 
 
 
 

int32_t exp2s (int log)
{
    uint32_t value;

    if (log < 0)
        return -exp2s (-log);

    value = exp2_table [log & 0xff] | 0x100;

    if ((log >>= 8) <= 9)
        return value >> (9 - log);
    else
        return value << (log - 9);
}

 
 
 

int restore_weight (signed char weight)
{
    int result;

    if ((result = (int) weight << 3) > 0)
        result += (result + 64) >> 7;

    return result;
}
