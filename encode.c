#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#define MAGIC_STRING "#*"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    // return secret file size
    fseek(fptr,0,SEEK_END);
    uint size = ftell(fptr);
    fseek(fptr,0,SEEK_SET);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // step 1 -> check argv[2] is having .bmp or not
        //yes -> store the file name into encInfo -> src_image_fname = argv[2]
        //no - > return e_failure
        int len;
        len = strlen(argv[2]);
        if(len<4)
        {
            return e_failure;
        }
        if(strcmp(argv[2] + len - 4, ".bmp")==0)
        {
            encInfo->src_image_fname = argv[2];
        }
        else
        {
            return e_failure;
        }

    // step 2 -> check argv[3] is having extn there or not
        // yes - > store the file name into encInfo -> secret_fname = argv[3]
        // no -> return e_failure
        if(strrchr(argv[3], '.') != NULL)
        {
            encInfo->secret_fname=argv[3];
            strcpy(encInfo->extn_secret_file, strrchr(argv[3], '.'));
        }
        else{
            return e_failure;
        }
    
    // step 3 -> check argv[4] is NULL or not
        //yes -> check argv[4] is having .bmp or not
                //yes -> store the file name into encInfo -> stego_image_fname = argv[4]
                //no - > return e_failure
        if(argv[4] != NULL)
        {
            int len4=strlen(argv[4]);
            if(len4 >= 4 && strcmp(argv[4] + len4-4, ".bmp")==0)
            {
                encInfo->stego_image_fname = argv[4];
            }
            else
            {
                return e_failure;
            }
        }
        
        //no -> store default name the encInfo -> stego_image_fname = "stego.bmp"
        else{
            encInfo->stego_image_fname = "stego.bmp";
        }


    // return e_success
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    fseek(encInfo->fptr_secret, 0, SEEK_END);
    encInfo->size_secret_file = ftell(encInfo->fptr_secret);
    rewind(encInfo->fptr_secret);

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success 
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    //step 1 -> encInfo -> image_capacity = call get_image_size_for_bmp(encoInfo -> fptr_src_image)
     encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    //step 2 -> encInfo -> size_secret_file = call get_file_size(encInfo -> fptr_secret)
     encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    //step 3 -> check encInfo -> image_capacity > 16 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8)
                //t -> return e_success;
                //f -> return e_failure
    if(encInfo->image_capacity > (16+32+(strlen(encInfo->extn_secret_file) * 8)+ 32 + (encInfo->size_secret_file * 8)))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // step 1 -> rewind source file pointer
     rewind(fptr_src_image);

    // step 2 -> read 54 bytes from source file
      char header[54];
      if(fread(header,1,54,fptr_src_image) != 54)
      {
        return e_failure;
      }

    // step 3 -> write 54 bytes to dest file
       if(fwrite(header,1,54,fptr_dest_image) != 54)
      {
        return e_failure;
      }

    // return e_success
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
     char buffer[8];

for(int i=0;i<strlen(magic_string);i++)
{
    //step 1 -> read 8 bytes from source file
    if(fread(buffer,1,8,encInfo->fptr_src_image) != 8)
    {
        return e_failure;
    }

    //step 2 -> call encode_byte_to_lsb(magic_string[i], buffer)
    encode_byte_to_lsb(magic_string[i], buffer);

    //step 3 -> write the buffer into dest file
    if(fwrite(buffer,1,8,encInfo->fptr_stego_image) != 8)
    {
        return e_failure;
    }
    
    // repeat this for strlen(magic_string) times from step 1
}
return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];

    if(fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    encode_size_to_lsb(size, buffer);

    if(fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for(int i=0;file_extn[i]!='\0';i++)
    {

    //step 1 -> read 8 bytes from source file
    if(fread(buffer,1,8,encInfo->fptr_src_image) != 8)
    {
        return e_failure;
    }

    //step 2 -> call encode_byte_to_lsb(file_extn[i], buffer)
    encode_byte_to_lsb(file_extn[i],buffer);

    //step 3 -> write the buffer into dest file
    if(fwrite(buffer,1,8,encInfo->fptr_stego_image) != 8)
    {
        return e_failure;
    }
    }
    return e_success;

    // repeat this for strlen(file_extn) times from step 1   
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
     char buffer[32];

    //step 1 -> read 32 bytes from source file
    if(fread(buffer,1,32,encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    //step 2 -> call encode_size_to_lsb(file_size, buffer)
    encode_size_to_lsb(file_size,buffer);

    //step 3 -> write the buffer into dest file
    if(fwrite(buffer,1,32,encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
    //step 1 -> read secret data from file and store it into encInfo -> secret_data
    char ch1=fgetc(encInfo->fptr_secret);

    // char buffer[8];
    char buffer[8];

    //step 2 -> read 8 bytes from source file
    fread(buffer,1,8,encInfo->fptr_src_image);

    //step 3 -> call encode_byte_to_lsb(encInfo -> secret_data[i], buffer)
    encode_byte_to_lsb(ch1,buffer); 

    //step 4 -> write the buffer into dest file
    fwrite(buffer,1,8,encInfo->fptr_stego_image); 
    }
    return e_success;

    // repeat this for strlen(encInfo -> size_secret_file) times from step 2
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    // write logic for copy remaining data
    char buffer[1024];
    size_t bytes;

    while((bytes = fread(buffer,1,sizeof(buffer),fptr_src)) > 0)
    {
        fwrite(buffer,1,bytes,fptr_dest);
    }

    // return e_success
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // write logic to encode the char
    for(int i=0;i<8;i++)
    {
        image_buffer[i]=image_buffer[i] & ~1;
    }
    for(int i=0;i<8;i++)
    {
        char ch1 = data & (1<<(7-i));
        if(ch1)
        {
            image_buffer[i]=image_buffer[i] | 1;
        }
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        imageBuffer[i] = imageBuffer[i] & 0xFE;

        if (size & (1 << (31 - i)))
        {
            imageBuffer[i] = imageBuffer[i] | 1;
        }
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //step 1 -> check open_files(encInfo) is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
        if(open_files(encInfo) == e_failure)
        {
            printf("Error : Unable to open files\n");
            return e_failure;
        }
        printf("Success : Files opened successfully\n");

    //step 2 -> check check_capacity(encInfo) is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
            if(check_capacity(encInfo) == e_failure)
            {
                printf("Error: Image doesn't have enough capacity\n");
                return e_failure;
            }
            printf("Success: Image has enough capacity\n");

    //step 3 -> call copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
            if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
            {
                printf("Error: Unable to copy BMP header\n");
                return e_failure;
            }
            printf("Success: BMP header copied successfully\n");

    //step 4 -> call encode_magic_string(MAGIC_STRING, encInfo)
     if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
     {
        printf("Error: Unable to encode magic string\n");
        return e_failure;
     }
     printf("Success: Magic string encoded successfully\n");
     
    //step 5 -> call encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo)
    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_failure)
     {
        printf("Error: Unable to encode secret file extension size\n");
        return e_failure;
     }
     printf("Success: Secret file extension size encoded successfully\n");

    //step 6 -> call encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)
     if(encode_secret_file_extn(encInfo-> extn_secret_file, encInfo) == e_failure)
     {
        printf("Error: Unable to encode secret file extension\n");
        return e_failure;
     }
     printf("Success: Secret file extension encoded successfully\n");

    //step 7 -> call encode_secret_file_size(encInfo ->  size_secret_file, encInfo)
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_failure)
     {
        printf("Error: Unable to encode secret file size\n");
        return e_failure;
     }
     printf("Success: Secret file size encoded successfully\n");

    //step 8 -> call encode_secret_file_data(encInfo)
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("Error: Unable to encode secret file data\n");
        return e_failure;
    }
    printf("Success: Secret file data encoded successfully\n");

    //step 9 -> copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success
            if(copy_remaining_img_data(encInfo-> fptr_src_image, encInfo->fptr_stego_image) == e_failure)
            {
                printf("Error: Unable to copy remaining image data\n");
                return e_failure;
            }
            printf("Success: Remaining image data copied successfully\n");
    // return e_success
    return e_success;
        
}