 
 
 
 
 
 
 

 

 
 
 
 

#include "wavpack.h"

#include <stdlib.h>
#include <string.h>

#define LOSSY_MUTE

 

 
 
 
 
 

int unpack_init (WavpackContext *wpc)
{
    WavpackStream *wps = &wpc->stream;
    WavpackMetadata wpmd;

    if (wps->wphdr.block_samples && wps->wphdr.block_index != (uint32_t) -1)
        wps->sample_index = wps->wphdr.block_index;

    wps->mute_error = FALSE;
    wps->crc = 0xffffffff;
    CLEAR (wps->wvbits);
    CLEAR (wps->decorr_passes);
    CLEAR (wps->w);

    while (read_metadata_buff (wpc, &wpmd)) {
        if (!process_metadata (wpc, &wpmd)) {
            strcpy (wpc->error_message, "invalid metadata!");
            return FALSE;
        }

        if (wpmd.id == ID_WV_BITSTREAM)
            break;
    }

    if (wps->wphdr.block_samples && !bs_is_open (&wps->wvbits)) {
        strcpy (wpc->error_message, "invalid WavPack file!");
        return FALSE;
    }

    if (wps->wphdr.block_samples) {
        if ((wps->wphdr.flags & INT32_DATA) && wps->int32_sent_bits)
            wpc->lossy_blocks = TRUE;

        if ((wps->wphdr.flags & FLOAT_DATA) &&
            wps->float_flags & (FLOAT_EXCEPTIONS | FLOAT_ZEROS_SENT | FLOAT_SHIFT_SENT | FLOAT_SHIFT_SAME))
                wpc->lossy_blocks = TRUE;
    }

    return TRUE;
}

 
 

int init_wv_bitstream (WavpackContext *wpc, WavpackMetadata *wpmd)
{
    WavpackStream *wps = &wpc->stream;

    if (wpmd->data)
        bs_open_read (&wps->wvbits, wpmd->data, (unsigned char *) wpmd->data + wpmd->byte_length, NULL, 0);
    else if (wpmd->byte_length)
        bs_open_read (&wps->wvbits, wpc->read_buffer, wpc->read_buffer + sizeof (wpc->read_buffer),
            wpc->infile, wpmd->byte_length + (wpmd->byte_length & 1));

    return TRUE;
}

 
 
 
 
 
 

int read_decorr_terms (WavpackStream *wps, WavpackMetadata *wpmd)
{
    int termcnt = wpmd->byte_length;
    uchar *byteptr = wpmd->data;
    struct decorr_pass *dpp;

    if (termcnt > MAX_NTERMS)
        return FALSE;

    wps->num_terms = termcnt;

    for (dpp = wps->decorr_passes + termcnt - 1; termcnt--; dpp--) {
        dpp->term = (int)(*byteptr & 0x1f) - 5;
        dpp->delta = (*byteptr++ >> 5) & 0x7;

        if (!dpp->term || dpp->term < -3 || (dpp->term > MAX_TERM && dpp->term < 17) || dpp->term > 18)
            return FALSE;
    }

    return TRUE;
}

 
 
 
 
 

int read_decorr_weights (WavpackStream *wps, WavpackMetadata *wpmd)
{
    int termcnt = wpmd->byte_length, tcount;
    signed char *byteptr = wpmd->data;
    struct decorr_pass *dpp;

    if (!(wps->wphdr.flags & MONO_DATA))
        termcnt /= 2;

    if (termcnt > wps->num_terms)
        return FALSE;

    for (tcount = wps->num_terms, dpp = wps->decorr_passes; tcount--; dpp++)
        dpp->weight_A = dpp->weight_B = 0;

    while (--dpp >= wps->decorr_passes && termcnt--) {
        dpp->weight_A = restore_weight (*byteptr++);

        if (!(wps->wphdr.flags & MONO_DATA))
            dpp->weight_B = restore_weight (*byteptr++);
    }

    return TRUE;
}

 
 
 
 
 
 
 

int read_decorr_samples (WavpackStream *wps, WavpackMetadata *wpmd)
{
    uchar *byteptr = wpmd->data;
    uchar *endptr = byteptr + wpmd->byte_length;
    struct decorr_pass *dpp;
    int tcount;

    for (tcount = wps->num_terms, dpp = wps->decorr_passes; tcount--; dpp++) {
        CLEAR (dpp->samples_A);
        CLEAR (dpp->samples_B);
    }

    if (wps->wphdr.version == 0x402 && (wps->wphdr.flags & HYBRID_FLAG)) {
        byteptr += 2;

        if (!(wps->wphdr.flags & MONO_DATA))
            byteptr += 2;
    }

    while (dpp-- > wps->decorr_passes && byteptr < endptr)
        if (dpp->term > MAX_TERM) {
            dpp->samples_A [0] = exp2s ((short)(byteptr [0] + (byteptr [1] << 8)));
            dpp->samples_A [1] = exp2s ((short)(byteptr [2] + (byteptr [3] << 8)));
            byteptr += 4;

            if (!(wps->wphdr.flags & MONO_DATA)) {
                dpp->samples_B [0] = exp2s ((short)(byteptr [0] + (byteptr [1] << 8)));
                dpp->samples_B [1] = exp2s ((short)(byteptr [2] + (byteptr [3] << 8)));
                byteptr += 4;
            }
        }
        else if (dpp->term < 0) {
            dpp->samples_A [0] = exp2s ((short)(byteptr [0] + (byteptr [1] << 8)));
            dpp->samples_B [0] = exp2s ((short)(byteptr [2] + (byteptr [3] << 8)));
            byteptr += 4;
        }
        else {
            int m = 0, cnt = dpp->term;

            while (cnt--) {
                dpp->samples_A [m] = exp2s ((short)(byteptr [0] + (byteptr [1] << 8)));
                byteptr += 2;

                if (!(wps->wphdr.flags & MONO_DATA)) {
                    dpp->samples_B [m] = exp2s ((short)(byteptr [0] + (byteptr [1] << 8)));
                    byteptr += 2;
                }

                m++;
            }
        }

    return byteptr == endptr;
}

 
 
 

int read_int32_info (WavpackStream *wps, WavpackMetadata *wpmd)
{
    int bytecnt = wpmd->byte_length;
    char *byteptr = wpmd->data;

    if (bytecnt != 4)
        return FALSE;

    wps->int32_sent_bits = *byteptr++;
    wps->int32_zeros = *byteptr++;
    wps->int32_ones = *byteptr++;
    wps->int32_dups = *byteptr;
    return TRUE;
}

 
 
 

int read_channel_info (WavpackContext *wpc, WavpackMetadata *wpmd)
{
    int bytecnt = wpmd->byte_length, shift = 0;
    char *byteptr = wpmd->data;
    uint32_t mask = 0;

    if (!bytecnt || bytecnt > 5)
        return FALSE;

    wpc->config.num_channels = *byteptr++;

    while (--bytecnt) {
        mask |= (uint32_t) *byteptr++ << shift;
        shift += 8;
    }

    wpc->config.channel_mask = mask;
    return TRUE;
}

 

int read_config_info (WavpackContext *wpc, WavpackMetadata *wpmd)
{
    int bytecnt = wpmd->byte_length;
    uchar *byteptr = wpmd->data;

    if (bytecnt >= 3) {
        wpc->config.flags &= 0xff;
        wpc->config.flags |= (int32_t) *byteptr++ << 8;
        wpc->config.flags |= (int32_t) *byteptr++ << 16;
        wpc->config.flags |= (int32_t) *byteptr << 24;
    }

    return TRUE;
}

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

#if defined(CPU_COLDFIRE) && !defined(SIMULATOR)
extern void decorr_stereo_pass_cont_mcf5249 (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count);
#elif defined(CPU_ARM) && !defined(SIMULATOR)
extern void decorr_stereo_pass_cont_arm (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count);
extern void decorr_stereo_pass_cont_arml (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count);
#else
static void decorr_stereo_pass_cont (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count);
#endif

static void decorr_mono_pass (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count);
static void decorr_stereo_pass (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count);
static void fixup_samples (WavpackStream *wps, int32_t *buffer, uint32_t sample_count);

int32_t unpack_samples (WavpackContext *wpc, int32_t *buffer, uint32_t sample_count)
{
    WavpackStream *wps = &wpc->stream;
    uint32_t flags = wps->wphdr.flags, crc = wps->crc, i;
    int32_t mute_limit = (1L << ((flags & MAG_MASK) >> MAG_LSB)) + 2;
    struct decorr_pass *dpp;
    int32_t *bptr, *eptr;
    int tcount;

    if (wps->sample_index + sample_count > wps->wphdr.block_index + wps->wphdr.block_samples)
        sample_count = wps->wphdr.block_index + wps->wphdr.block_samples - wps->sample_index;

    if (wps->mute_error) {
        memset (buffer, 0, sample_count * (flags & MONO_FLAG ? 4 : 8));
        wps->sample_index += sample_count;
        return sample_count;
    }

    if (flags & HYBRID_FLAG)
        mute_limit *= 2;

     

    if (flags & MONO_DATA) {
        eptr = buffer + sample_count;
        i = get_words (buffer, sample_count, flags, &wps->w, &wps->wvbits);

        for (tcount = wps->num_terms, dpp = wps->decorr_passes; tcount--; dpp++)
            decorr_mono_pass (dpp, buffer, sample_count);

        for (bptr = buffer; bptr < eptr; ++bptr) {
            if (labs (bptr [0]) > mute_limit) {
                i = bptr - buffer;
                break;
            }

            crc = crc * 3 + bptr [0];
        }
    }

     

    else {
        eptr = buffer + (sample_count * 2);
        i = get_words (buffer, sample_count, flags, &wps->w, &wps->wvbits);

        if (sample_count < 16)
            for (tcount = wps->num_terms, dpp = wps->decorr_passes; tcount--; dpp++)
                decorr_stereo_pass (dpp, buffer, sample_count);
        else
            for (tcount = wps->num_terms, dpp = wps->decorr_passes; tcount--; dpp++) {
                decorr_stereo_pass (dpp, buffer, 8);
#if defined(CPU_COLDFIRE) && !defined(SIMULATOR)
                decorr_stereo_pass_cont_mcf5249 (dpp, buffer + 16, sample_count - 8);
#elif defined(CPU_ARM) && !defined(SIMULATOR)
                if (((flags & MAG_MASK) >> MAG_LSB) > 15)
                    decorr_stereo_pass_cont_arml (dpp, buffer + 16, sample_count - 8);
                else
                    decorr_stereo_pass_cont_arm (dpp, buffer + 16, sample_count - 8);
#else
                decorr_stereo_pass_cont (dpp, buffer + 16, sample_count - 8);
#endif
            }

        if (flags & JOINT_STEREO)
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                bptr [0] += (bptr [1] -= (bptr [0] >> 1));

                if (labs (bptr [0]) > mute_limit || labs (bptr [1]) > mute_limit) {
                    i = (bptr - buffer) / 2;
                    break;
                }

                crc = (crc * 3 + bptr [0]) * 3 + bptr [1];
            }
        else
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                if (labs (bptr [0]) > mute_limit || labs (bptr [1]) > mute_limit) {
                    i = (bptr - buffer) / 2;
                    break;
                }

                crc = (crc * 3 + bptr [0]) * 3 + bptr [1];
            }
    }

    if (i != sample_count) {
        memset (buffer, 0, sample_count * (flags & MONO_FLAG ? 4 : 8));
        wps->mute_error = TRUE;
        i = sample_count;
    }

    fixup_samples (wps, buffer, i);

    if (flags & FALSE_STEREO) {
        int32_t *dptr = buffer + i * 2;
        int32_t *sptr = buffer + i;
        int32_t c = i;

        while (c--) {
            *--dptr = *--sptr;
            *--dptr = *sptr;
        }
    }

    wps->sample_index += i;
    wps->crc = crc;

    return i;
}

static void decorr_stereo_pass (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count)
{
    int32_t delta = dpp->delta, weight_A = dpp->weight_A, weight_B = dpp->weight_B;
    int32_t *bptr, *eptr = buffer + (sample_count * 2), sam_A, sam_B;
    int m, k;

    switch (dpp->term) {

        case 17:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                sam_A = 2 * dpp->samples_A [0] - dpp->samples_A [1];
                dpp->samples_A [1] = dpp->samples_A [0];
                dpp->samples_A [0] = apply_weight (weight_A, sam_A) + bptr [0];
                update_weight (weight_A, delta, sam_A, bptr [0]);
                bptr [0] = dpp->samples_A [0];

                sam_A = 2 * dpp->samples_B [0] - dpp->samples_B [1];
                dpp->samples_B [1] = dpp->samples_B [0];
                dpp->samples_B [0] = apply_weight (weight_B, sam_A) + bptr [1];
                update_weight (weight_B, delta, sam_A, bptr [1]);
                bptr [1] = dpp->samples_B [0];
            }

            break;

        case 18:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                sam_A = (3 * dpp->samples_A [0] - dpp->samples_A [1]) >> 1;
                dpp->samples_A [1] = dpp->samples_A [0];
                dpp->samples_A [0] = apply_weight (weight_A, sam_A) + bptr [0];
                update_weight (weight_A, delta, sam_A, bptr [0]);
                bptr [0] = dpp->samples_A [0];

                sam_A = (3 * dpp->samples_B [0] - dpp->samples_B [1]) >> 1;
                dpp->samples_B [1] = dpp->samples_B [0];
                dpp->samples_B [0] = apply_weight (weight_B, sam_A) + bptr [1];
                update_weight (weight_B, delta, sam_A, bptr [1]);
                bptr [1] = dpp->samples_B [0];
            }

            break;

        default:
            for (m = 0, k = dpp->term & (MAX_TERM - 1), bptr = buffer; bptr < eptr; bptr += 2) {
                sam_A = dpp->samples_A [m];
                dpp->samples_A [k] = apply_weight (weight_A, sam_A) + bptr [0];
                update_weight (weight_A, delta, sam_A, bptr [0]);
                bptr [0] = dpp->samples_A [k];

                sam_A = dpp->samples_B [m];
                dpp->samples_B [k] = apply_weight (weight_B, sam_A) + bptr [1];
                update_weight (weight_B, delta, sam_A, bptr [1]);
                bptr [1] = dpp->samples_B [k];

                m = (m + 1) & (MAX_TERM - 1);
                k = (k + 1) & (MAX_TERM - 1);
            }

            if (m) {
                int32_t temp_samples [MAX_TERM];

                memcpy (temp_samples, dpp->samples_A, sizeof (dpp->samples_A));

                for (k = 0; k < MAX_TERM; k++, m++)
                    dpp->samples_A [k] = temp_samples [m & (MAX_TERM - 1)];

                memcpy (temp_samples, dpp->samples_B, sizeof (dpp->samples_B));

                for (k = 0; k < MAX_TERM; k++, m++)
                    dpp->samples_B [k] = temp_samples [m & (MAX_TERM - 1)];
            }

            break;

        case -1:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                sam_A = bptr [0] + apply_weight (weight_A, dpp->samples_A [0]);
                update_weight_clip (weight_A, delta, dpp->samples_A [0], bptr [0]);
                bptr [0] = sam_A;
                dpp->samples_A [0] = bptr [1] + apply_weight (weight_B, sam_A);
                update_weight_clip (weight_B, delta, sam_A, bptr [1]);
                bptr [1] = dpp->samples_A [0];
            }

            break;

        case -2:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                sam_B = bptr [1] + apply_weight (weight_B, dpp->samples_B [0]);
                update_weight_clip (weight_B, delta, dpp->samples_B [0], bptr [1]);
                bptr [1] = sam_B;
                dpp->samples_B [0] = bptr [0] + apply_weight (weight_A, sam_B);
                update_weight_clip (weight_A, delta, sam_B, bptr [0]);
                bptr [0] = dpp->samples_B [0];
            }

            break;

        case -3:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                sam_A = bptr [0] + apply_weight (weight_A, dpp->samples_A [0]);
                update_weight_clip (weight_A, delta, dpp->samples_A [0], bptr [0]);
                sam_B = bptr [1] + apply_weight (weight_B, dpp->samples_B [0]);
                update_weight_clip (weight_B, delta, dpp->samples_B [0], bptr [1]);
                bptr [0] = dpp->samples_B [0] = sam_A;
                bptr [1] = dpp->samples_A [0] = sam_B;
            }

            break;
    }

    dpp->weight_A = weight_A;
    dpp->weight_B = weight_B;
}

#if (!defined(CPU_COLDFIRE) && !defined(CPU_ARM)) || defined(SIMULATOR)

static void decorr_stereo_pass_cont (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count)
{
    int32_t delta = dpp->delta, weight_A = dpp->weight_A, weight_B = dpp->weight_B;
    int32_t *bptr, *tptr, *eptr = buffer + (sample_count * 2), sam_A, sam_B;
    int k, i;

    switch (dpp->term) {

        case 17:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                sam_A = 2 * bptr [-2] - bptr [-4];
                bptr [0] = apply_weight (weight_A, sam_A) + (sam_B = bptr [0]);
                update_weight (weight_A, delta, sam_A, sam_B);

                sam_A = 2 * bptr [-1] - bptr [-3];
                bptr [1] = apply_weight (weight_B, sam_A) + (sam_B = bptr [1]);
                update_weight (weight_B, delta, sam_A, sam_B);
            }

            dpp->samples_B [0] = bptr [-1];
            dpp->samples_A [0] = bptr [-2];
            dpp->samples_B [1] = bptr [-3];
            dpp->samples_A [1] = bptr [-4];
            break;

        case 18:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                sam_A = (3 * bptr [-2] - bptr [-4]) >> 1;
                bptr [0] = apply_weight (weight_A, sam_A) + (sam_B = bptr [0]);
                update_weight (weight_A, delta, sam_A, sam_B);

                sam_A = (3 * bptr [-1] - bptr [-3]) >> 1;
                bptr [1] = apply_weight (weight_B, sam_A) + (sam_B = bptr [1]);
                update_weight (weight_B, delta, sam_A, sam_B);
            }

            dpp->samples_B [0] = bptr [-1];
            dpp->samples_A [0] = bptr [-2];
            dpp->samples_B [1] = bptr [-3];
            dpp->samples_A [1] = bptr [-4];
            break;

        default:
            for (bptr = buffer, tptr = buffer - (dpp->term * 2); bptr < eptr; bptr += 2, tptr += 2) {
                bptr [0] = apply_weight (weight_A, tptr [0]) + (sam_A = bptr [0]);
                update_weight (weight_A, delta, tptr [0], sam_A);

                bptr [1] = apply_weight (weight_B, tptr [1]) + (sam_A = bptr [1]);
                update_weight (weight_B, delta, tptr [1], sam_A);
            }

            for (k = dpp->term - 1, i = 8; i--; k--) {
                dpp->samples_B [k & (MAX_TERM - 1)] = *--bptr;
                dpp->samples_A [k & (MAX_TERM - 1)] = *--bptr;
            }

            break;

        case -1:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                bptr [0] = apply_weight (weight_A, bptr [-1]) + (sam_A = bptr [0]);
                update_weight_clip (weight_A, delta, bptr [-1], sam_A);
                bptr [1] = apply_weight (weight_B, bptr [0]) + (sam_A = bptr [1]);
                update_weight_clip (weight_B, delta, bptr [0], sam_A);
            }

            dpp->samples_A [0] = bptr [-1];
            break;

        case -2:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                bptr [1] = apply_weight (weight_B, bptr [-2]) + (sam_A = bptr [1]);
                update_weight_clip (weight_B, delta, bptr [-2], sam_A);
                bptr [0] = apply_weight (weight_A, bptr [1]) + (sam_A = bptr [0]);
                update_weight_clip (weight_A, delta, bptr [1], sam_A);
            }

            dpp->samples_B [0] = bptr [-2];
            break;

        case -3:
            for (bptr = buffer; bptr < eptr; bptr += 2) {
                bptr [0] = apply_weight (weight_A, bptr [-1]) + (sam_A = bptr [0]);
                update_weight_clip (weight_A, delta, bptr [-1], sam_A);
                bptr [1] = apply_weight (weight_B, bptr [-2]) + (sam_A = bptr [1]);
                update_weight_clip (weight_B, delta, bptr [-2], sam_A);
            }

            dpp->samples_A [0] = bptr [-1];
            dpp->samples_B [0] = bptr [-2];
            break;
    }

    dpp->weight_A = weight_A;
    dpp->weight_B = weight_B;
}

#endif

static void decorr_mono_pass (struct decorr_pass *dpp, int32_t *buffer, int32_t sample_count)
{
    int32_t delta = dpp->delta, weight_A = dpp->weight_A;
    int32_t *bptr, *eptr = buffer + sample_count, sam_A;
    int m, k;

    switch (dpp->term) {

        case 17:
            for (bptr = buffer; bptr < eptr; bptr++) {
                sam_A = 2 * dpp->samples_A [0] - dpp->samples_A [1];
                dpp->samples_A [1] = dpp->samples_A [0];
                dpp->samples_A [0] = apply_weight (weight_A, sam_A) + bptr [0];
                update_weight (weight_A, delta, sam_A, bptr [0]);
                bptr [0] = dpp->samples_A [0];
            }

            break;

        case 18:
            for (bptr = buffer; bptr < eptr; bptr++) {
                sam_A = (3 * dpp->samples_A [0] - dpp->samples_A [1]) >> 1;
                dpp->samples_A [1] = dpp->samples_A [0];
                dpp->samples_A [0] = apply_weight (weight_A, sam_A) + bptr [0];
                update_weight (weight_A, delta, sam_A, bptr [0]);
                bptr [0] = dpp->samples_A [0];
            }

            break;

        default:
            for (m = 0, k = dpp->term & (MAX_TERM - 1), bptr = buffer; bptr < eptr; bptr++) {
                sam_A = dpp->samples_A [m];
                dpp->samples_A [k] = apply_weight (weight_A, sam_A) + bptr [0];
                update_weight (weight_A, delta, sam_A, bptr [0]);
                bptr [0] = dpp->samples_A [k];
                m = (m + 1) & (MAX_TERM - 1);
                k = (k + 1) & (MAX_TERM - 1);
            }

            if (m) {
                int32_t temp_samples [MAX_TERM];

                memcpy (temp_samples, dpp->samples_A, sizeof (dpp->samples_A));

                for (k = 0; k < MAX_TERM; k++, m++)
                    dpp->samples_A [k] = temp_samples [m & (MAX_TERM - 1)];
            }

            break;
    }

    dpp->weight_A = weight_A;
}


 
 
 
 
 
 
 

static void fixup_samples (WavpackStream *wps, int32_t *buffer, uint32_t sample_count)
{
    uint32_t flags = wps->wphdr.flags;
    int shift = (flags & SHIFT_MASK) >> SHIFT_LSB;

    if (flags & FLOAT_DATA) {
        float_values (wps, buffer, (flags & MONO_FLAG) ? sample_count : sample_count * 2);
        return;
    }

    if (flags & INT32_DATA) {
        uint32_t count = (flags & MONO_FLAG) ? sample_count : sample_count * 2;
        int sent_bits = wps->int32_sent_bits, zeros = wps->int32_zeros;
        int ones = wps->int32_ones, dups = wps->int32_dups;
        int32_t *dptr = buffer;

        if (!(flags & HYBRID_FLAG) && !sent_bits && (zeros + ones + dups))
            while (count--) {
                if (zeros)
                    *dptr <<= zeros;
                else if (ones)
                    *dptr = ((*dptr + 1) << ones) - 1;
                else if (dups)
                    *dptr = ((*dptr + (*dptr & 1)) << dups) - (*dptr & 1);

                dptr++;
            }
        else
            shift += zeros + sent_bits + ones + dups;
    }

    if (flags & HYBRID_FLAG) {
        int32_t min_value, max_value, min_shifted, max_shifted;

        switch (flags & BYTES_STORED) {
            case 0:
                min_shifted = (min_value = -128 >> shift) << shift;
                max_shifted = (max_value = 127 >> shift) << shift;
                break;

            case 1:
                min_shifted = (min_value = -32768 >> shift) << shift;
                max_shifted = (max_value = 32767 >> shift) << shift;
                break;

            case 2:
                min_shifted = (min_value = -8388608 >> shift) << shift;
                max_shifted = (max_value = 8388607 >> shift) << shift;
                break;

            case 3:
            default:
                min_shifted = (min_value = (int32_t) 0x80000000 >> shift) << shift;
                max_shifted = (max_value = (int32_t) 0x7FFFFFFF >> shift) << shift;
                break;
        }

        if (!(flags & MONO_FLAG))
            sample_count *= 2;

        while (sample_count--) {
            if (*buffer < min_value)
                *buffer++ = min_shifted;
            else if (*buffer > max_value)
                *buffer++ = max_shifted;
            else
                *buffer++ <<= shift;
        }
    }
    else if (shift) {
        if (!(flags & MONO_FLAG))
            sample_count *= 2;

        while (sample_count--)
            *buffer++ <<= shift;
    }
}

 
 
 
 
 
 

int check_crc_error (WavpackContext *wpc)
{
    WavpackStream *wps = &wpc->stream;
    int result = 0;

    if (wps->crc != wps->wphdr.crc)
        ++result;

    return result;
}
