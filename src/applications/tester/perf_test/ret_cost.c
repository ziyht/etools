#include "etype.h"

/**
  * using valgrind to checkit
  *
  */

#pragma pack(1)
typedef struct u96_s  { u32  v1; u64  v2; } u96;
typedef struct u128_s { u64  v1; u64  v2; } u128;
typedef struct u256_s { u128 v1; u128 v2; } u256;
#pragma pack()


static u8   ret_u8  () { u8   v; return v; }    // 2.48%
static u16  ret_u16 () { u16  v; return v; }    // 2.48%
static u32  ret_u32 () { u32  v; return v; }    // 2.48%
static u64  ret_u64 () { u64  v; return v; }    // 2.48%
static u96  ret_u96 () { u96  v; return v; }    // 5.46%
static u128 ret_u128() { u128 v; return v; }    // 2.98%    <-- best, more data and cost a little
static u256 ret_u256() { u256 v; return v; }    // 7.44%

static void ret_u8_iterate  (int loop) { u8   v; while(loop) {loop--; v = ret_u8  ();} }    // 5.46%
static void ret_u16_iterate (int loop) { u16  v; while(loop) {loop--; v = ret_u16 ();} }    // 5.46%
static void ret_u32_iterate (int loop) { u32  v; while(loop) {loop--; v = ret_u32 ();} }    // 5.46%
static void ret_u64_iterate (int loop) { u64  v; while(loop) {loop--; v = ret_u64 ();} }    // 5.46%
static void ret_u96_iterate (int loop) { u96  v; while(loop) {loop--; v = ret_u96 ();} }    // 8.93%
static void ret_u128_iterate(int loop) { u128 v; while(loop) {loop--; v = ret_u128();} }    // 6.45%
static void ret_u256_iterate(int loop) { u256 v; while(loop) {loop--; v = ret_u256();} }    // 10.90%

static u8   ret_u8_callback  (int cnt) { if(!cnt){ u8   v; return v; } return ret_u8_callback  (--cnt);  }    // 5.95%
static u16  ret_u16_callback (int cnt) { if(!cnt){ u16  v; return v; } return ret_u16_callback (--cnt);  }    // 5.95%
static u32  ret_u32_callback (int cnt) { if(!cnt){ u32  v; return v; } return ret_u32_callback (--cnt);  }    // 5.95%
static u64  ret_u64_callback (int cnt) { if(!cnt){ u64  v; return v; } return ret_u64_callback (--cnt);  }    // 5.95%
static u96  ret_u96_callback (int cnt) { if(!cnt){ u96  v; return v; } return ret_u96_callback (--cnt);  }    // 7.94%
static u128 ret_u128_callback(int cnt) { if(!cnt){ u128 v; return v; } return ret_u128_callback(--cnt);  }    // 5.95%   <-- best, more data and cost a little
static u256 ret_u256_callback(int cnt) { if(!cnt){ u256 v; return v; } return ret_u256_callback(--cnt);  }    // 7.94%

int ret_cost(int argc, char* argv[])    // 93.7%
{
    int loop = 100000;

    ret_u8_iterate(loop);
    ret_u16_iterate(loop);
    ret_u32_iterate(loop);
    ret_u64_iterate(loop);

    ret_u96_iterate(loop);
    ret_u128_iterate(loop);
    ret_u256_iterate(loop);

    int layer = 100000;
    ret_u8_callback(layer);
    ret_u16_callback(layer);
    ret_u32_callback(layer);
    ret_u64_callback(layer);
    ret_u96_callback(layer);
    ret_u128_callback(layer);
    ret_u256_callback(layer);

    return ETEST_OK;
}
