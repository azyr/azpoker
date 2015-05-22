import itertools
import numpy as np
from libcpp.string cimport string
from libcpp.vector cimport vector

ctypedef long long int64

cdef extern from "peval.hpp":
    void c_evaluate_high "evaluate_high" (int64* masks,int numevals, int* out)
    double c_evaluate_high_perm "evaluate_high_perm" (int64 hc1, int64 board)

def evaluate_high(int64[:] masks, int[:] out):
    c_evaluate_high(&masks[0], masks.shape[0], &out[0])

def evaluate_high_perm(int64 hc1, int64 board):
    return c_evaluate_high_perm(hc1, board)

def handmask_to_codes(int64 mask):
    codes = []
    cdef int64 i
    for i in range(52):
        testbit = 1L << i
        if mask & testbit:
            codes.append(i)
    return np.array(codes, dtype=np.int64)

def codes_to_mask(codes):
    handmask = 0
    for code in codes:
        mask = 1 << code
        handmask |= mask
    return handmask

def calc_permutations(list deck, int picks):
    res = [codes_to_mask(x) for x in itertools.combinations(deck, picks)]
    return res
