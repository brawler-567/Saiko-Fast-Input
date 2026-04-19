 
 
 
 
 
 
 

 

 
 
 
 
 
 

#include "wavpack.h"

#include <string.h>

 

const uint32_t sample_rates [] = { 6000, 8000, 9600, 11025, 12000, 16000, 22050,
    24000, 32000, 44100, 48000, 64000, 88200, 96000, 192000 };

 

static uint32_t read_next_header (read_stream infile, WavpackHeader *wphdr);
        
 
 
 
 
 
 
 
 
 
 
 
 
 
 

static WavpackContext wpc;

WavpackContext *WavpackOpenFileInput (read_stream infile, char *error)
{
    WavpackStream *wps = &wpc.stream;
    uint32_t bcount;

    CLEAR (wpc);
    wpc.infile = infile;
    wpc.total_samples = (uint32_t) -1;
    wpc.norm_offset = 0;
    wpc.open_flags = 0;

     

    while (!wps->wphdr.block_samples) {

        bcount = read_next_header (wpc.infile, &wps->wphdr);

        if (bcount == (uint32_t) -1) {
            strcpy (error, "not compatible with this version of WavPack file!");
            return NULL;
        }

        if (wps->wphdr.block_samples && wps->wphdr.total_samples != (uint32_t) -1)
            wpc.total_samples = wps->wphdr.total_samples;

        if (!unpack_init (&wpc)) {
            strcpy (error, wpc.error_message [0] ? wpc.error_message :
                "not compatible with this version of WavPack file!");

            return NULL;
        }
    }

    wpc.config.flags &= ~0xff;
    wpc.config.flags |= wps->wphdr.flags & 0xff;
    wpc.config.bytes_per_sample = (wps->wphdr.flags & BYTES_STORED) + 1;
    wpc.config.float_norm_exp = wps->float_norm_exp;

    wpc.config.bits_per_sample = (wpc.config.bytes_per_sample * 8) - 
        ((wps->wphdr.flags & SHIFT_MASK) >> SHIFT_LSB);

    if (wpc.config.flags & FLOAT_DATA) {
        wpc.config.bytes_per_sample = 3;
        wpc.config.bits_per_sample = 24;
    }

    if (!wpc.config.sample_rate) {
        if (!wps || !wps->wphdr.block_samples || (wps->wphdr.flags & SRATE_MASK) == SRATE_MASK)
            wpc.config.sample_rate = 44100;
        else
            wpc.config.sample_rate = sample_rates [(wps->wphdr.flags & SRATE_MASK) >> SRATE_LSB];
    }

    if (!wpc.config.num_channels) {
        wpc.config.num_channels = (wps->wphdr.flags & MONO_FLAG) ? 1 : 2;
        wpc.config.channel_mask = 0x5 - wpc.config.num_channels;
    }

    if (!(wps->wphdr.flags & FINAL_BLOCK))
        wpc.reduced_channels = (wps->wphdr.flags & MONO_FLAG) ? 1 : 2;

    return &wpc;
}

 
 

 
 
 
 
 
 

int WavpackGetMode (WavpackContext *wpc)
{
    int mode = 0;

    if (wpc) {
        if (wpc->config.flags & CONFIG_HYBRID_FLAG)
            mode |= MODE_HYBRID;
        else if (!(wpc->config.flags & CONFIG_LOSSY_MODE))
            mode |= MODE_LOSSLESS;

        if (wpc->lossy_blocks)
            mode &= ~MODE_LOSSLESS;

        if (wpc->config.flags & CONFIG_FLOAT_DATA)
            mode |= MODE_FLOAT;

        if (wpc->config.flags & CONFIG_HIGH_FLAG)
            mode |= MODE_HIGH;

        if (wpc->config.flags & CONFIG_FAST_FLAG)
            mode |= MODE_FAST;
    }

    return mode;
}

 
 
 
 
 
 
 
 
 

uint32_t WavpackUnpackSamples (WavpackContext *wpc, int32_t *buffer, uint32_t samples)
{
    WavpackStream *wps = &wpc->stream;
    uint32_t bcount, samples_unpacked = 0, samples_to_unpack;
    int num_channels = wpc->config.num_channels;

    while (samples) {
        if (!wps->wphdr.block_samples || !(wps->wphdr.flags & INITIAL_BLOCK) ||
            wps->sample_index >= wps->wphdr.block_index + wps->wphdr.block_samples) {
                bcount = read_next_header (wpc->infile, &wps->wphdr);

                if (bcount == (uint32_t) -1)
                    break;

                if (!wps->wphdr.block_samples || wps->sample_index == wps->wphdr.block_index)
                    if (!unpack_init (wpc))
                        break;
        }

        if (!wps->wphdr.block_samples || !(wps->wphdr.flags & INITIAL_BLOCK) ||
            wps->sample_index >= wps->wphdr.block_index + wps->wphdr.block_samples)
                continue;

        if (wps->sample_index < wps->wphdr.block_index) {
            samples_to_unpack = wps->wphdr.block_index - wps->sample_index;

            if (samples_to_unpack > samples)
                samples_to_unpack = samples;

            wps->sample_index += samples_to_unpack;
            samples_unpacked += samples_to_unpack;
            samples -= samples_to_unpack;

            if (wpc->reduced_channels)
                samples_to_unpack *= wpc->reduced_channels;
            else
                samples_to_unpack *= num_channels;

            while (samples_to_unpack--)
                *buffer++ = 0;

            continue;
        }

        samples_to_unpack = wps->wphdr.block_index + wps->wphdr.block_samples - wps->sample_index;

        if (samples_to_unpack > samples)
            samples_to_unpack = samples;

        unpack_samples (wpc, buffer, samples_to_unpack);

        if (wpc->reduced_channels)
            buffer += samples_to_unpack * wpc->reduced_channels;
        else
            buffer += samples_to_unpack * num_channels;

        samples_unpacked += samples_to_unpack;
        samples -= samples_to_unpack;

        if (wps->sample_index == wps->wphdr.block_index + wps->wphdr.block_samples) {
            if (check_crc_error (wpc))
                wpc->crc_errors++;
        }

        if (wps->sample_index == wpc->total_samples)
            break;
    }

    return samples_unpacked;
}

 

uint32_t WavpackGetNumSamples (WavpackContext *wpc)
{
    return wpc ? wpc->total_samples : (uint32_t) -1;
}

 

uint32_t WavpackGetSampleIndex (WavpackContext *wpc)
{
    if (wpc)
        return wpc->stream.sample_index;

    return (uint32_t) -1;
}

 

int WavpackGetNumErrors (WavpackContext *wpc)
{
    return wpc ? wpc->crc_errors : 0;
}

 

int WavpackLossyBlocks (WavpackContext *wpc)
{
    return wpc ? wpc->lossy_blocks : 0;
}

 

uint32_t WavpackGetSampleRate (WavpackContext *wpc)
{
    return wpc ? wpc->config.sample_rate : 44100;
}

 
 
 

int WavpackGetNumChannels (WavpackContext *wpc)
{
    return wpc ? wpc->config.num_channels : 2;
}

 
 
 
 
 
 
 

int WavpackGetBitsPerSample (WavpackContext *wpc)
{
    return wpc ? wpc->config.bits_per_sample : 16;
}

 
 
 
 

int WavpackGetBytesPerSample (WavpackContext *wpc)
{
    return wpc ? wpc->config.bytes_per_sample : 2;
}

 
 
 
 

int WavpackGetReducedChannels (WavpackContext *wpc)
{
    if (wpc)
        return wpc->reduced_channels ? wpc->reduced_channels : wpc->config.num_channels;
    else
        return 2;
}

 
 
 
 
 

static uint32_t read_next_header (read_stream infile, WavpackHeader *wphdr)
{
    char buffer [sizeof (*wphdr)], *sp = buffer + sizeof (*wphdr), *ep = sp;
    uint32_t bytes_skipped = 0;
    int bleft;

    while (1) {
        if (sp < ep) {
            bleft = ep - sp;
            memcpy (buffer, sp, bleft);
        }
        else
            bleft = 0;

        if (infile (buffer + bleft, sizeof (*wphdr) - bleft) != (int32_t) sizeof (*wphdr) - bleft)
            return -1;

        sp = buffer;

        if (*sp++ == 'w' && *sp == 'v' && *++sp == 'p' && *++sp == 'k' &&
            !(*++sp & 1) && sp [2] < 16 && !sp [3] && sp [5] == 4 &&
            sp [4] >= (MIN_STREAM_VERS & 0xff) && sp [4] <= (MAX_STREAM_VERS & 0xff)) {
                memcpy (wphdr, buffer, sizeof (*wphdr));
                little_endian_to_native (wphdr, WavpackHeaderFormat);
                return bytes_skipped;
            }

        while (sp < ep && *sp != 'w')
            sp++;

        if ((bytes_skipped += sp - buffer) > 1048576L)
            return -1;
    }
}
