import itertools
import numpy as np
from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "stovewrapper.h":
    int c_evaluate_high "evaluate_high" (long long* masks,int numevals, int* out)

def evaluate_high(long long[:] masks, int[:] out):
    c_evaluate_high(&masks[0], masks.shape[0], &out[0])

def handmask_to_codes(mask):
    codes = []
    for i in range(52):
        testbit = 1 << i
        if mask & testbit:
            codes.append(i)
    return codes

def codes_to_mask(codes):
    handmask = 0
    for code in codes:
        mask = 1 << code
        handmask |= mask
    return handmask

def calc_permutations(list deck, int picks):
    res = [codes_to_mask(x) for x in itertools.combinations(deck, picks)]
    return res
