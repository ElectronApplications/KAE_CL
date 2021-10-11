#include "sha256.cl"

__constant const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void hexstr(__private char *dest, __private char *src) {
    for(int i = 0; i < 32; ++i) {
        dest[2 * i]     = hexmap[(src[i] & 0xF0) >> 4];
        dest[2 * i + 1] = hexmap[src[i] & 0x0F];
    }
}

bool strcmp(__global const char *str1, __private const char *str2) {
    for(int i = 0; i < 64; i++) {
        if(str1[i] != str2[i])
            return false;
    }
    return true;
}

typedef struct {
    unsigned int len;
    char data[16];
} input_t;

__kernel void miner(__global const input_t *input, __global const char *hash, __global int *index) {
    char buffer[32];
    char hexbuffer[64];

    hash_glbl_to_priv((__global unsigned int*) input[get_global_id(0)].data, input[get_global_id(0)].len, (__private unsigned int*) buffer);
    hexstr(hexbuffer, buffer);

    if(strcmp(hash, hexbuffer))
        *index = get_global_id(0);
    
}