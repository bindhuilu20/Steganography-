#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "decode.h"
#include "types.h"
#define MAGIC_STRING "#*"

/* Read and validate command line arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    /* Check stego image */
    if (argv[2] != NULL && strstr(argv[2], ".bmp") != NULL)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf("Error: Invalid stego image\n");
        return e_failure;
    }

    /* Output file name */
    if (argv[3] != NULL)
    {   
        decInfo->secret_fname = argv[3];
    }
    else
    {
        decInfo->secret_fname = "output";
    }

    return e_success;
}

/* Open stego image and output file */
Status decode_open_files(DecodeInfo *decInfo)
{
    /* Open stego image */
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        printf("Error: Unable to open stego image\n");
        return e_failure;
    }

    /* Open output secret file */
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");
    if (decInfo->fptr_secret == NULL)
    {
        printf("Error: Unable to open output file\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}

/* Check decoding capacity */
Status decode_check_capacity(DecodeInfo *decInfo)
{
    decInfo->image_capacity = get_image_size_for_bmp(decInfo->fptr_stego_image);
    return e_success;
}

/* Decode one byte from 8 image bytes */
char decode_byte(char *image_buffer)
{
    char data = 0;

    for (int i = 0; i < 8; i++)
    {
        data = data << 1;
        data = data | (image_buffer[i] & 1);
    }

    return data;
}

/* Decode integer from 32 image bytes */
int decode_size(char *image_buffer)
{
    int size = 0;

    for (int i = 0; i < 32; i++)
    {
        size = size << 1;
        size = size | (image_buffer[i] & 1);
    }

    return size;
}


/* Decode magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8];
    char decoded_char;

    int length = strlen(magic_string);

    for (int i = 0; i < length; i++)
    {
        if (fread(buffer, 8, 1,decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }

        decoded_char = decode_byte(buffer);

        if (decoded_char != magic_string[i])
        {
            return e_failure;
        }
    }

    return e_success;
}


/* Decode secret file extension size */
Status decode_secret_file_extn_size(int size, DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }

    size = decode_size(buffer);

    if (size <= 0 ||
        size >= sizeof(decInfo->extn_secret_file))
    {
        return e_failure;
    }

    decInfo->secret_file_extn_size = size;

    return e_success;
}

/* Decode secret file extension */
Status decode_secret_file_extn(const char *file_extn, DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    int size = strlen(file_extn);

    for (int i = 0; i < size; i++)
    {
        if (fread(buffer, 8, 1,decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }

        ch = decode_byte(buffer);

        decInfo->extn_secret_file[i] = ch;
    }

    decInfo->extn_secret_file[size] = '\0';

    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(long file_size, DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 32, 1,decInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }

    file_size = decode_size(buffer);

    decInfo->size_secret_file = file_size;

    return e_success;
}

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for (long i = 0;i < decInfo->size_secret_file;i++)
    {
        if (fread(buffer, 8, 1,decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }

        ch = decode_byte(buffer);

        if (fwrite(&ch, 1, 1,decInfo->fptr_secret) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}

/* Main decoding function */
Status do_decoding(DecodeInfo *decInfo)
{
    char buffer[32];

    int extn_size;
    long file_size;

    /* Step 1: Open files */
    if (decode_open_files(decInfo) == e_failure)
    {
        printf("Error: Failed to open files\n");
        return e_failure;
    }

    printf("Files opened successfully\n");

    /* Step 2: Check capacity */
    if (decode_check_capacity(decInfo) == e_failure)
    {
        printf("Error: Capacity check failed\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_failure;
    }

    printf("Capacity check successful\n");

    /* Step 3: Skip BMP header */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    /* Step 4: Decode magic string */
    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
    {
        printf("Error: Magic string mismatch\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_failure;
    }

    printf("Magic string decoded successfully\n");

    /* Step 5: Decode extension size */
    if (fread(buffer, 32, 1,decInfo->fptr_stego_image) != 1)
    {
        printf("Error: Failed to decode extension size\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_failure;
    }

    extn_size = decode_size(buffer);

    printf("Extension size = %d\n", extn_size);

    if (extn_size <= 0 ||
        extn_size >= sizeof(decInfo->extn_secret_file))
    {
        printf("Error: Invalid extension size\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_failure;
    }

    decInfo->secret_file_extn_size = extn_size;

    /* Step 6: Decode extension */
    for (int i = 0; i < extn_size; i++)
    {
        if (fread(buffer, 8, 1,decInfo->fptr_stego_image) != 1)
        {
            printf("Error: Failed to decode extension\n");

            fclose(decInfo->fptr_stego_image);
            fclose(decInfo->fptr_secret);

            return e_failure;
        }

        decInfo->extn_secret_file[i] = decode_byte((char *)buffer);
    }

    decInfo->extn_secret_file[extn_size] = '\0';

    printf("Secret extension = %s\n",decInfo->extn_secret_file);

    /* Step 7: Decode secret file size */
    if (fread(buffer, 32, 1,decInfo->fptr_stego_image) != 1)
    {
        printf("Error: Failed to decode file size\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_failure;
    }

    file_size = decode_size(buffer);

    decInfo->size_secret_file = file_size;

    printf("Secret file size = %ld\n",decInfo->size_secret_file);

    /* Step 8: Decode secret file data */
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf("Error: Failed to decode secret data\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_failure;
    }

    printf("Secret data decoded successfully\n");

    /* Close files */
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_secret);

    printf("Decoding completed successfully\n");

    return e_success;
}
