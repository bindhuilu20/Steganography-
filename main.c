#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include<string.h>


OperationType check_operation_type(char *symbol);

int main(int argc, char *argv[]) // .aout -e <source_file> <secret_data_file> <optional>
{
    //step 1 -> check check_operation_type(argv[1]) is returning e_encode or not
        // yes -> EncodeInfo encInfo;
            // check read_and_validate_encode_args(argv, &encInfo) is returning e_success or e_failure
                // failure -> print error msg and stop
                // success -> print success msg 
                            // check do_encoding(&encInfo) is returning e_success or e_failure
                                    // failure -> print error msg and stop
                                    // success -> print success msg and stop
    if(argc < 2)
    {
        printf("Error: Insufficient arguments\n");
        return e_failure;
    }
    if(check_operation_type(argv[1]) == e_encode)
    {
        EncodeInfo encInfo;

        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("Error: Invalid encoding arguments\n");
            return e_failure;
        }

        printf("Success: Encoding arguments successful\n");

        if (do_encoding(&encInfo) == e_failure)
        {
            printf("Error: Encoding failed\n");
            return e_failure;
        }

        printf("Success: Encoding successful\n");
    }
    
    else if(check_operation_type(argv[1]) == e_decode)
    {
        DecodeInfo decInfo;
        if (read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            printf("Error: Invalid decoding arguments\n");
            return e_failure;
        }

        printf("Success: Decoding arguments successful\n");

        if (do_decoding(&decInfo) == e_failure)
        {
            printf("Error: Decoding failed\n");
            return e_failure;
        }

        printf("Success: Decoding successful\n");
    }
    
    else
    {
        printf("Error: Unsupported opration\n");
        return e_failure;
    }
    return e_success;
}

OperationType check_operation_type(char *symbol)
{
    //step 1 -> check symbol is -e or not
        // yes -> return e_encode

    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    
    // step 2 -> check symbol is -d or not
        // yes -> return e_decode

    if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }

    // return e_unsupported
    return e_unsupported;
}
