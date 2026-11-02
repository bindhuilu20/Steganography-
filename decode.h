#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

#define MAGIC_STRING "#*"

typedef struct _DecodeInfo
{
    /* Stego image information */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output secret file information */
    char *secret_fname;
    FILE *fptr_secret;

    /* Image capacity */
    long image_capacity;

    /* Secret file extension */
    char extn_secret_file[5];

    /* Secret file extension size */
    int secret_file_extn_size;

    /* Secret file size */
    long size_secret_file;

} DecodeInfo;


/* Read and validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open stego image and output file */
Status decode_open_files(DecodeInfo *decInfo);

/* Decode magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(int size, DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(const char *file_extn, DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(long file_size, DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode byte from LSB */
char decode_byte(char *image_buffer);

/* Decode integer from LSB */
int decode_size(char *image_buffer);

/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Common helper functions */
uint get_image_size_for_bmp(FILE *fptr_image);
uint get_file_size(FILE *fptr);

#endif   