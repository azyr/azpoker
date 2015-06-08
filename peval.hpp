// #include <string>
// #include <vector>
#include <iostream>
#include <bitset>
#include <boost/shared_ptr.hpp>
#include "pokerstove/peval/PokerHandEvaluator.h"
#include "pokerstove/peval/CardSet.h"

using namespace std;
using namespace pokerstove;

typedef long long int64;

template<typename T>
void show_binrep(const T& a)
{
    const char* beg = reinterpret_cast<const char*>(&a);
    const char* end = beg + sizeof(a);
    while(beg != end)
        std::cout << std::bitset<CHAR_BIT>(*beg++) << ' ';
    std::cout << '\n';
}


// the fastest hamming weight algo adopted from http://en.wikipedia.org/wiki/Hamming_weight
const int64 M1  = 0x5555555555555555; //binary: 0101...
const int64 M2  = 0x3333333333333333; //binary: 00110011..
const int64 M4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const int64 H01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...
int popcount(int64 x)
{
    x -= (x >> 1L) & M1;             //put count of each 2 bits into those 2 bits
    x = (x & M2) + ((x >> 2L) & M2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4L)) & M4;        //put count of each 8 bits into those 8 bits 
    return (x * H01) >> 56L;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}

// from http://graphics.stanford.edu/~seander/bithacks.html
int64 bh_next_perm_fast(int64 v)
{
    int64 t = v | (v - 1L);
    // __builtin_ctzl is a gcc specific intrinsic
    int64 w = (t + 1L) | (((~t & -~t) - 1L) >> (__builtin_ctzl(v) + 1L));  
    return w;
}

int64 element_0(int64 c)
{
    int64 res = (1L << c) - 1L;
    return res;
}

int64 calc_permutations(int p, int b, int64* out)
{
    int64 v, initial, block_mask;
    v = initial = element_0(p);
    block_mask = element_0(b);
    out[0] = v;

    int64 i = 1;
    while (v >= initial)
    {
        v = bh_next_perm_fast(v) & block_mask;
        out[i++] = v;
    }
    return i - 1L;
}

int64 insert_one(int64 x, int64 loc)
{
    int64 lowmask = (1L << loc) - 1L;
    int64 highmask = 0xFFFFFFFFFFFFFFFF ^ lowmask;
    return ((x & highmask) << 1L) | (x & lowmask) | (1L << loc);
}

int64 insert_zero(int64 x, int64 loc)
{
    int64 lowmask = (1L << loc) - 1L;
    int64 highmask = 0xFFFFFFFFFFFFFFFF ^ lowmask;
    return ((x & highmask) << 1L) | (x & lowmask);
}

void get_bit_locations(int64 x, int64 nbits, int64* out)
{
    int64 hits = 0L;
    for (int64 i = 0L; i < 52L; ++i)
    {
        int64 testbit = 1L << i;
        if (x & testbit)
        {
            out[hits++] = i;
            if (hits == nbits) break;
        }
    }
}

double evaluate_high_perm(int64 hc1, int64 board, bool rs)
{
    int64 cnt;

    // compute number of cards
    int64 board_ncards = popcount(board);
    int64 hc1_ncards = popcount(hc1);
    int64 ncards = board_ncards + hc1_ncards;

    // compute bitmask permutations (all the possible hands)
    int64 masks[2000];  // 108x should be the max amount ever needed ... increase if neccessary
    int64 tot_perms = calc_permutations(2, 50L - board_ncards, &masks[1]) + 1L;
    masks[0] = hc1 | board;
    
    // reserved bits
    int64 bit_locs[10];
    bool bit_is_one[10];

    // compute reserved bits
    cnt = 0L;
    for (int64 i = 0L; i < 52L; ++i)
    {
        int64 testbit = 1L << i;
        if (hc1 & testbit)
        {
            bit_locs[cnt] = i;
            bit_is_one[cnt] = false;
            ++cnt;
            if (cnt == ncards) break;
        }
        else if (board & testbit)
        {
            bit_locs[cnt] = i;
            bit_is_one[cnt] = true;
            ++cnt;
            if (cnt == ncards) break;
        }
    }

    // apply reserved bits to masks
    for (int64 i1 = 1L; i1 < tot_perms; ++i1)
    {
        for (int64 i2 = 0L; i2 < ncards; ++i2)
        {
            int64 loc = bit_locs[i2];
            if (bit_is_one[i2])
            {
                masks[i1] = insert_one(masks[i1], loc);
            }
            else
            {
                masks[i1] = insert_zero(masks[i1], loc);
            }
        }
    }

    // compute hand values
    int hvals[2000];
    for (int64 i = 0L; i < tot_perms; ++i)
    {
        CardSet h(masks[i]);
        int hval = h.evaluateHigh().code();
        // cout << h.str() << " -> " << hval << endl;
        hvals[i] = hval;
    }

    // compute percentile
    double dcnt = 0.0;
    if (!rs)
    {
        for (int64 i = 1L; i < tot_perms; ++i)
        {
            if (hvals[0] >= hvals[i])
            {
                dcnt += 1.0;
            }
        }
    }
    else
    {
        for (int64 i = 1L; i < tot_perms; ++i)
        {
            if (hvals[0] > hvals[i])
            {
                dcnt += 1.0;
            }
            else if (hvals[0] == hvals[i])
            {
                dcnt += 0.5;
            }
        }
    }
    // cout << tot_perms << endl;
    // cout << hc1_ncards << " " << board_ncards << endl;
    return dcnt / (tot_perms - 1L);
}

int evaluate_high(int64* masks, int numevals, int* out)
{
    // boost::shared_ptr<PokerHandEvaluator> peval = PokerHandEvaluator::alloc('h');

    for (int i = 0; i < numevals; ++i)
    {
        CardSet h(masks[i]);
        // PokerHandEvaluation handval = peval->evaluate(holecards[i], boards[i]);
        // out[i] = handval.high().code();
        int hval = h.evaluateHigh().code();
        // cout << h.str() << " -> " << hval << endl;
        out[i] = hval;
    }
    // cout << numevals << endl;
    return 0;
}

