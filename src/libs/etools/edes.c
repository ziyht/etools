/// =====================================================================================
///
///       Filename:  edes.c
///
///    Description:  an easier des encoder/decoder
///
///        Version:  0.8
///        Created:  03/03/2017 04:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "edes.h"

static int initial_key_permutaion[] = { 57, 49,  41, 33,  25,  17,  9,
                                         1, 58,  50, 42,  34,  26, 18,
                                        10,  2,  59, 51,  43,  35, 27,
                                        19, 11,   3, 60,  52,  44, 36,
                                        63, 55,  47, 39,  31,  23, 15,
                                         7, 62,  54, 46,  38,  30, 22,
                                        14,  6,  61, 53,  45,  37, 29,
                                        21, 13,   5, 28,  20,  12,  4};

static int initial_message_permutation[] =	  { 58, 50, 42, 34, 26, 18, 10, 2,
                                                60, 52, 44, 36, 28, 20, 12, 4,
                                                62, 54, 46, 38, 30, 22, 14, 6,
                                                64, 56, 48, 40, 32, 24, 16, 8,
                                                57, 49, 41, 33, 25, 17,  9, 1,
                                                59, 51, 43, 35, 27, 19, 11, 3,
                                                61, 53, 45, 37, 29, 21, 13, 5,
                                                63, 55, 47, 39, 31, 23, 15, 7};

static int key_shift_sizes[] = {-1, 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

static int sub_key_permutation[] =    { 14, 17, 11, 24,  1,  5,
                                         3, 28, 15,  6, 21, 10,
                                        23, 19, 12,  4, 26,  8,
                                        16,  7, 27, 20, 13,  2,
                                        41, 52, 31, 37, 47, 55,
                                        30, 40, 51, 45, 33, 48,
                                        44, 49, 39, 56, 34, 53,
                                        46, 42, 50, 36, 29, 32};

static int message_expansion[] =  { 32,  1,  2,  3,  4,  5,
                                     4,  5,  6,  7,  8,  9,
                                     8,  9, 10, 11, 12, 13,
                                    12, 13, 14, 15, 16, 17,
                                    16, 17, 18, 19, 20, 21,
                                    20, 21, 22, 23, 24, 25,
                                    24, 25, 26, 27, 28, 29,
                                    28, 29, 30, 31, 32,  1};

static int S1[] = { 14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
                     0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
                     4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
                    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13};

static int S2[] = { 15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
                     3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
                     0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
                    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9};

static int S3[] = { 10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
                    13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
                    13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
                     1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12};

static int S4[] = {  7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
                    13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
                    10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
                     3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14};

static int S5[] = {  2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
                    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
                     4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
                    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3};

static int S6[] = { 12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
                    10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
                     9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
                     4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13};

static int S7[] = {  4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
                    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
                     1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
                     6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12};

static int S8[] = { 13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
                     1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
                     7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
                     2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11};

static int right_sub_message_permutation[] =  { 16,  7, 20, 21,
                                                29, 12, 28, 17,
                                                 1, 15, 23, 26,
                                                 5, 18, 31, 10,
                                                 2,  8, 24, 14,
                                                32, 27,  3,  9,
                                                19, 13, 30,  6,
                                                22, 11,  4, 25};

static int final_message_permutation[] =  { 40,  8, 48, 16, 56, 24, 64, 32,
                                            39,  7, 47, 15, 55, 23, 63, 31,
                                            38,  6, 46, 14, 54, 22, 62, 30,
                                            37,  5, 45, 13, 53, 21, 61, 29,
                                            36,  4, 44, 12, 52, 20, 60, 28,
                                            35,  3, 43, 11, 51, 19, 59, 27,
                                            34,  2, 42, 10, 50, 18, 58, 26,
                                            33,  1, 41,  9, 49, 17, 57, 25};


typedef struct key_set_s{
    unsigned char k[8];
    unsigned char c[4];
    unsigned char d[4];
} key_set_t;

#define ENCRYPTION_MODE 1
#define DECRYPTION_MODE 0

void print_char_as_binary(char input) {
    int i;
    for (i=0; i<8; i++) {
        char shift_byte = 0x01 << (7-i);
        if (shift_byte & input) {
            printf("1");
        } else {
            printf("0");
        }
    }
}

void generate_key(unsigned char* key) {
    int i;
    for (i=0; i<8; i++) {
        key[i] = rand()%255;
    }
}

void print_key_set(key_set_t key_set){
    int i;
    printf("K: \n");
    for (i=0; i<8; i++) {
        printf("%02X : ", key_set.k[i]);
        print_char_as_binary(key_set.k[i]);
        printf("\n");
    }
    printf("\nC: \n");

    for (i=0; i<4; i++) {
        printf("%02X : ", key_set.c[i]);
        print_char_as_binary(key_set.c[i]);
        printf("\n");
    }
    printf("\nD: \n");

    for (i=0; i<4; i++) {
        printf("%02X : ", key_set.d[i]);
        print_char_as_binary(key_set.d[i]);
        printf("\n");
    }
    printf("\n");
}

void generate_sub_keys(const unsigned char* main_key, key_set_t* key_sets) {
    int i, j;
    int shift_size;
    unsigned char shift_byte, first_shift_bits, second_shift_bits, third_shift_bits, fourth_shift_bits;

    for (i=0; i<8; i++) {
        key_sets[0].k[i] = 0;
    }

    for (i=0; i<56; i++) {
        shift_size = initial_key_permutaion[i];
        shift_byte = 0x80 >> ((shift_size - 1)%8);
        shift_byte &= main_key[(shift_size - 1)/8];
        shift_byte <<= ((shift_size - 1)%8);

        key_sets[0].k[i/8] |= (shift_byte >> i%8);
    }

    for (i=0; i<3; i++) {
        key_sets[0].c[i] = key_sets[0].k[i];
    }

    key_sets[0].c[3] = key_sets[0].k[3] & 0xF0;

    for (i=0; i<3; i++) {
        key_sets[0].d[i] = (key_sets[0].k[i+3] & 0x0F) << 4;
        key_sets[0].d[i] |= (key_sets[0].k[i+4] & 0xF0) >> 4;
    }

    key_sets[0].d[3] = (key_sets[0].k[6] & 0x0F) << 4;


    for (i=1; i<17; i++) {
        for (j=0; j<4; j++) {
            key_sets[i].c[j] = key_sets[i-1].c[j];
            key_sets[i].d[j] = key_sets[i-1].d[j];
        }

        shift_size = key_shift_sizes[i];
        if (shift_size == 1){
            shift_byte = 0x80;
        } else {
            shift_byte = 0xC0;
        }

        // Process C
        first_shift_bits = shift_byte & key_sets[i].c[0];
        second_shift_bits = shift_byte & key_sets[i].c[1];
        third_shift_bits = shift_byte & key_sets[i].c[2];
        fourth_shift_bits = shift_byte & key_sets[i].c[3];

        key_sets[i].c[0] <<= shift_size;
        key_sets[i].c[0] |= (second_shift_bits >> (8 - shift_size));

        key_sets[i].c[1] <<= shift_size;
        key_sets[i].c[1] |= (third_shift_bits >> (8 - shift_size));

        key_sets[i].c[2] <<= shift_size;
        key_sets[i].c[2] |= (fourth_shift_bits >> (8 - shift_size));

        key_sets[i].c[3] <<= shift_size;
        key_sets[i].c[3] |= (first_shift_bits >> (4 - shift_size));

        // Process D
        first_shift_bits = shift_byte & key_sets[i].d[0];
        second_shift_bits = shift_byte & key_sets[i].d[1];
        third_shift_bits = shift_byte & key_sets[i].d[2];
        fourth_shift_bits = shift_byte & key_sets[i].d[3];

        key_sets[i].d[0] <<= shift_size;
        key_sets[i].d[0] |= (second_shift_bits >> (8 - shift_size));

        key_sets[i].d[1] <<= shift_size;
        key_sets[i].d[1] |= (third_shift_bits >> (8 - shift_size));

        key_sets[i].d[2] <<= shift_size;
        key_sets[i].d[2] |= (fourth_shift_bits >> (8 - shift_size));

        key_sets[i].d[3] <<= shift_size;
        key_sets[i].d[3] |= (first_shift_bits >> (4 - shift_size));

        for (j=0; j<48; j++) {
            shift_size = sub_key_permutation[j];
            if (shift_size <= 28) {
                shift_byte = 0x80 >> ((shift_size - 1)%8);
                shift_byte &= key_sets[i].c[(shift_size - 1)/8];
                shift_byte <<= ((shift_size - 1)%8);
            } else {
                shift_byte = 0x80 >> ((shift_size - 29)%8);
                shift_byte &= key_sets[i].d[(shift_size - 29)/8];
                shift_byte <<= ((shift_size - 29)%8);
            }

            key_sets[i].k[j/8] |= (shift_byte >> j%8);
        }
    }
}

void process_message(unsigned char* message_piece, unsigned char* processed_piece, key_set_t* key_sets, int mode) {
    int i, k;
    int shift_size;
    unsigned char shift_byte;

    unsigned char initial_permutation[8];
    memset(initial_permutation, 0, 8);
    memset(processed_piece, 0, 8);

    for (i=0; i<64; i++) {
        shift_size = initial_message_permutation[i];
        shift_byte = 0x80 >> ((shift_size - 1)%8);
        shift_byte &= message_piece[(shift_size - 1)/8];
        shift_byte <<= ((shift_size - 1)%8);

        initial_permutation[i/8] |= (shift_byte >> i%8);
    }

    unsigned char l[4], r[4];
    for (i=0; i<4; i++) {
        l[i] = initial_permutation[i];
        r[i] = initial_permutation[i+4];
    }

    unsigned char ln[4], rn[4], er[6], ser[4];

    int key_index;
    for (k=1; k<=16; k++) {
        memcpy(ln, r, 4);

        memset(er, 0, 6);

        for (i=0; i<48; i++) {
            shift_size = message_expansion[i];
            shift_byte = 0x80 >> ((shift_size - 1)%8);
            shift_byte &= r[(shift_size - 1)/8];
            shift_byte <<= ((shift_size - 1)%8);

            er[i/8] |= (shift_byte >> i%8);
        }

        if (mode == DECRYPTION_MODE) {
            key_index = 17 - k;
        } else {
            key_index = k;
        }

        for (i=0; i<6; i++) {
            er[i] ^= key_sets[key_index].k[i];
        }

        unsigned char row, column;

        for (i=0; i<4; i++) {
            ser[i] = 0;
        }

        // 0000 0000 0000 0000 0000 0000
        // rccc crrc cccr rccc crrc cccr

        // Byte 1
        row = 0;
        row |= ((er[0] & 0x80) >> 6);
        row |= ((er[0] & 0x04) >> 2);

        column = 0;
        column |= ((er[0] & 0x78) >> 3);

        ser[0] |= ((unsigned char)S1[row*16+column] << 4);

        row = 0;
        row |= (er[0] & 0x02);
        row |= ((er[1] & 0x10) >> 4);

        column = 0;
        column |= ((er[0] & 0x01) << 3);
        column |= ((er[1] & 0xE0) >> 5);

        ser[0] |= (unsigned char)S2[row*16+column];

        // Byte 2
        row = 0;
        row |= ((er[1] & 0x08) >> 2);
        row |= ((er[2] & 0x40) >> 6);

        column = 0;
        column |= ((er[1] & 0x07) << 1);
        column |= ((er[2] & 0x80) >> 7);

        ser[1] |= ((unsigned char)S3[row*16+column] << 4);

        row = 0;
        row |= ((er[2] & 0x20) >> 4);
        row |= (er[2] & 0x01);

        column = 0;
        column |= ((er[2] & 0x1E) >> 1);

        ser[1] |= (unsigned char)S4[row*16+column];

        // Byte 3
        row = 0;
        row |= ((er[3] & 0x80) >> 6);
        row |= ((er[3] & 0x04) >> 2);

        column = 0;
        column |= ((er[3] & 0x78) >> 3);

        ser[2] |= ((unsigned char)S5[row*16+column] << 4);

        row = 0;
        row |= (er[3] & 0x02);
        row |= ((er[4] & 0x10) >> 4);

        column = 0;
        column |= ((er[3] & 0x01) << 3);
        column |= ((er[4] & 0xE0) >> 5);

        ser[2] |= (unsigned char)S6[row*16+column];

        // Byte 4
        row = 0;
        row |= ((er[4] & 0x08) >> 2);
        row |= ((er[5] & 0x40) >> 6);

        column = 0;
        column |= ((er[4] & 0x07) << 1);
        column |= ((er[5] & 0x80) >> 7);

        ser[3] |= ((unsigned char)S7[row*16+column] << 4);

        row = 0;
        row |= ((er[5] & 0x20) >> 4);
        row |= (er[5] & 0x01);

        column = 0;
        column |= ((er[5] & 0x1E) >> 1);

        ser[3] |= (unsigned char)S8[row*16+column];

        for (i=0; i<4; i++) {
            rn[i] = 0;
        }

        for (i=0; i<32; i++) {
            shift_size = right_sub_message_permutation[i];
            shift_byte = 0x80 >> ((shift_size - 1)%8);
            shift_byte &= ser[(shift_size - 1)/8];
            shift_byte <<= ((shift_size - 1)%8);

            rn[i/8] |= (shift_byte >> i%8);
        }

        for (i=0; i<4; i++) {
            rn[i] ^= l[i];
        }

        for (i=0; i<4; i++) {
            l[i] = ln[i];
            r[i] = rn[i];
        }
    }

    unsigned char pre_end_permutation[8];
    for (i=0; i<4; i++) {
        pre_end_permutation[i] = r[i];
        pre_end_permutation[4+i] = l[i];
    }

    for (i=0; i<64; i++) {
        shift_size = final_message_permutation[i];
        shift_byte = 0x80 >> ((shift_size - 1)%8);
        shift_byte &= pre_end_permutation[(shift_size - 1)/8];
        shift_byte <<= ((shift_size - 1)%8);

        processed_piece[i/8] |= (shift_byte >> i%8);
    }
}

/// -------------------------- micros helper ---------------------------------

#define exe_ret(expr, ret ) { expr;      return ret;}
#define is0_ret(cond, ret ) if(!(cond)){ return ret;}
#define is1_ret(cond, ret ) if( (cond)){ return ret;}
#define is0_exe(cond, expr) if(!(cond)){ expr;}
#define is1_exe(cond, expr) if( (cond)){ expr;}

#define is0_exeret(cond, expr, ret) if(!(cond)){ expr;        return ret;}
#define is1_exeret(cond, expr, ret) if( (cond)){ expr;        return ret;}
#define is0_elsret(cond, expr, ret) if(!(cond)){ expr;} else{ return ret;}
#define is1_elsret(cond, expr, ret) if( (cond)){ expr;} else{ return ret;}

static int __processb2b(constr key, constr in, size inlen, cstr out, size* outlen, int mode)
{
    u32 block_idx, block_all; u8 padding, * rd_p, * wr_p; u8 pin[8], pout[8]; key_set_t key_sets[17] = {{{0},{0},{0}},};

    // -- Generate DES key set
    generate_sub_keys((u8*)key, key_sets);

    // -- Get number of blocks in the file and init hit
    block_all = inlen/8 + ((inlen%8)?1:0);
    block_idx = 0;
    rd_p      = (u8*)in;
    wr_p      = (u8*)out;

    // -- ready? let's do it
    while(1)
    {
        block_idx++;

        if(block_idx < block_all)
        {
            memcpy(pin , rd_p, 8); rd_p += 8;
            process_message(pin, pout, key_sets, mode);
            memcpy(wr_p, pout, 8); wr_p += 8;

            continue;
        }

        // -- is the last block? do some special operation
        if (mode == ENCRYPTION_MODE)
        {
            padding = 8 - inlen%8;

            memcpy(pin, rd_p, 8 - padding % 8);

            if (padding < 8)                    // Fill empty data block bytes with padding
                memset(pin + 8 - padding, padding, padding);


            process_message(pin, pout, key_sets, mode);
            memcpy(wr_p, pout, 8); wr_p += 8;

            if(padding == 8)                    // Write an extra block for padding, (padding = 8)
            {
                memset(pin, padding, 8);
                process_message(pin, pout, key_sets, mode);
                memcpy(wr_p, pout, 8); wr_p += 8;
            }
        }
        else
        {
            memcpy(pin, rd_p, 8);
            process_message(pin, pout, key_sets, mode);
            padding = pout[7];

            if (padding < 8)
            {
                memcpy(wr_p, pout, 8 - padding); wr_p += 8 - padding;
            }
        }

        break;
    }

    *outlen = wr_p - (u8*)out;

    return 1;
}

/*
static void __processb2f(constr key, constr in, size inlen, constr out, int type)
{
    unsigned long file_size;
    unsigned short int padding;

    // Generate DES key set
    short int bytes_written, process_mode;
    unsigned long block_count = 0, number_of_blocks;
    unsigned char* data_block = (unsigned char*) malloc(8*sizeof(char));
    unsigned char* processed_block = (unsigned char*) malloc(8*sizeof(char));
    key_set_t* key_sets = (key_set_t*)malloc(17*sizeof(key_set_t));

    generate_sub_keys(key, key_sets);


    // Get number of blocks in the file


    number_of_blocks = file_size/8 + ((file_size%8)?1:0);

    start = clock();

    // Start reading input file, process and write to output file
    while(fread(data_block, 1, 8, input_file)) {
        block_count++;
        if (block_count == number_of_blocks) {
            if (process_mode == ENCRYPTION_MODE) {
                padding = 8 - file_size%8;
                if (padding < 8) { // Fill empty data block bytes with padding
                    memset((data_block + 8 - padding), (unsigned char)padding, padding);
                }

                process_message(data_block, processed_block, key_sets, process_mode);
                bytes_written = fwrite(processed_block, 1, 8, output_file);

                if (padding == 8) { // Write an extra block for padding
                    memset(data_block, (unsigned char)padding, 8);
                    process_message(data_block, processed_block, key_sets, process_mode);
                    bytes_written = fwrite(processed_block, 1, 8, output_file);
                }
            } else {
                process_message(data_block, processed_block, key_sets, process_mode);
                padding = processed_block[7];

                if (padding < 8) {
                    bytes_written = fwrite(processed_block, 1, 8 - padding, output_file);
                }
            }
        } else {
            process_message(data_block, processed_block, key_sets, process_mode);
            bytes_written = fwrite(processed_block, 1, 8, output_file);
        }
        memset(data_block, 0, 8);
    }

    finish = clock();

    // Free up memory
    free(des_key);
    free(data_block);
    free(processed_block);
    fclose(input_file);
    fclose(output_file);
}
*/

/// ------------------------ win32 API setting -------------------------------
#if (_WIN32)
#define inline
#endif

/// ----------------------------- edes ---------------------------------------

cstr edes_genkey(char key[8], int human)
{
    static char key_set[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static uint  seed;
    int i;

    srand((unsigned) time(0) + seed);

    seed += (uint)time(0);

    if(human) for(i=0; i<8; i++) key[i] = key_set[rand()%(sizeof(key_set) - 1)];
    else      for(i=0; i<8; i++) key[i] = rand()%255;

    return key;
}

static  u64 __EDES_INNER_MAGIC_NUM = 0xFDFEFFEEEEFFFEFDLL;
#define EDES_INNER_MAGIC_KEY (char*)&__EDES_INNER_MAGIC_NUM

static inline void __rand_key(cstr key) { edes_genkey(key, 0); key[7] = '\0'; }

/// ---------------------- encoder -------------------------

estr edes_encb  (constr key, conptr in, size inlen)
{
    estr out; size outlen; char real_key[8];

    is0_ret(key, 0); is0_ret(in, 0); is0_ret(inlen, 0);

    is0_ret(out = estr_newLen(0, 8 + inlen + 8), 0);

#if EDES_SAFE_CODEC

    do{
        __rand_key(real_key);
        __processb2b(key, real_key, 7, out, &outlen, ENCRYPTION_MODE);
    }while(0 == memcmp(out, EDES_INNER_MAGIC_KEY, 8));

#else

    memcpy(out, EDES_INNER_MAGIC_KEY, 8);
    memcpy(real_key, key, 8);

#endif

    __processb2b(real_key, in, inlen, out + 8, &outlen, ENCRYPTION_MODE);
    estr_incrLen(out, 8 + outlen);

    return out;
}

int  edes_encb2b(constr key, conptr in, size inlen, cptr out, size* outlen)
{
    char real_key[8];

    is0_ret(key, 0); is0_ret(in, 0); is0_ret(inlen, 0); is0_ret(out, 0); is0_ret(outlen, 0);

#if EDES_SAFE_CODEC

    do{
        __rand_key(real_key);
        __processb2b(key, real_key, 7, out, outlen, ENCRYPTION_MODE);
    }while(0 == memcmp(out, EDES_INNER_MAGIC_KEY, 8));

#else

    memcpy(out, EDES_INNER_MAGIC_KEY, 8);
    memcpy(real_key, key, 8);

#endif

    __processb2b(real_key, in, inlen, (cstr)out + 8, outlen, ENCRYPTION_MODE);
    *outlen += 8;

    return 1;
}

/// ---------------------- decoder -------------------------

estr edes_decb  (constr key, conptr in, size inlen)
{
    estr out; size outlen;

    is0_ret(key, 0); is0_ret(in, 0); is0_ret(inlen, 0);

    is0_ret(out = estr_newLen(0, inlen - 8), 0);

    if(0 != memcmp(in, EDES_INNER_MAGIC_KEY, 8))    // using safe mode
    {
        char real_key[8];

        __processb2b(key, in, 8, real_key, &outlen, DECRYPTION_MODE); real_key[7] = '\0';

        __processb2b(real_key, (char*)in + 8, inlen - 8, out, &outlen, DECRYPTION_MODE);
        estr_incrLen(out, outlen);
    }
    else
    {
        __processb2b(key, (char*)in + 8, inlen - 8, out, &outlen, DECRYPTION_MODE);
        estr_incrLen(out, outlen);
    }

    return out;

}

int  edes_decb2b(constr key, conptr in, size inlen, cptr out, size* outlen)
{
    is0_ret(key, 0); is0_ret(in, 0); is0_ret(inlen, 0); is0_ret(out, 0); is0_ret(outlen, 0);

    if(0 != memcmp(in, EDES_INNER_MAGIC_KEY, 8))    // using safe mode
    {
        char real_key[8];

        __processb2b(key, in, 8, real_key, outlen, DECRYPTION_MODE); real_key[7] = '\0';

        __processb2b(real_key, (cstr)in + 8, inlen - 8, out, outlen, DECRYPTION_MODE);
    }
    else
    {
        __processb2b(key, (cstr)in + 8, inlen - 8, out, outlen, DECRYPTION_MODE);

    }

    return 1;
}

/// ----------------------- utils --------------------------

inline void   edes_show(estr s) {        estr_show(s); }
inline size   edes_dlen(estr s) { return estr_len (s); }
inline void   edes_free(estr s) {        estr_free(s); }

estr edes_version()
{
    static char buf[16];
    static sstr ver;

    if(!ver)
    {
        ver = sstr_init(buf, sizeof(buf) -1);
        sstr_wrtS(ver, EDES_VERSION);
    }

    return ver;
}
