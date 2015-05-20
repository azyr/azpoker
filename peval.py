import numpy as np
import itertools
from azpoker import peval_ex

SUIT_CODE = {
    'c': 0,
    'd': 1,
    'h': 2,
    's': 3
}
RANK_CODE = {
    '2': 0,
    '3': 1,
    '4': 2,
    '5': 3,
    '6': 4,
    '7': 5,
    '8': 6,
    '9': 7,
    'T': 8,
    'J': 9,
    'Q': 10,
    'K': 11,
    'A': 12
}
ALL_CARDS = sorted(itertools.product(RANK_CODE.keys(), SUIT_CODE.keys()),
                   key=lambda x: RANK_CODE[x[0]] + SUIT_CODE[x[1]] * len(RANK_CODE))
ALL_CARDS = ["".join(x) for x in ALL_CARDS]
CARD_TO_CODE = {x: RANK_CODE[x[0]] + SUIT_CODE[x[1]] * len(RANK_CODE) for x in ALL_CARDS}
CODE_TO_CARD = {code: card for card, code in CARD_TO_CODE.items()}

def card_to_code(s):
    return RANK_CODE[s[0]] + SUIT_CODE[s[1]] * len(RANK_CODE)

def cards_to_codes(s):
    res = []
    for i in range(0, len(s), 2):
        res.append(card_to_code(s[i:i+2]))
    return res

def codes_to_mask(codes):
    handmask = 0
    for code in codes:
        mask = 1 << code
        handmask |= mask
    return handmask

def strhand_to_mask(s):
    handmask = 0
    for i in range(0, len(s), 2):
        code = card_to_code(s[i:i+2])
        mask = 1 << code
        if mask & handmask:
            raise Exception("Duplicate cards: {}".format(s))
        # print(code, "{0:b}".format(mask), "{0:b}".format(handmask))
        handmask |= mask
    return handmask

def handmask_to_codes(mask):
    codes = []
    for i in range(52):
        testbit = 1 << i
        if mask & testbit:
            codes.append(i)
    return codes

def handmask_to_str(mask):
    return "".join([CODE_TO_CARD[x] for x in handmask_to_codes(mask)])

def rank_flushdraw(fv, ncards):
    fv_code = RANK_CODE[fv]
    assert fv_code > RANK_CODE['2']
    assert 1 <= ncards <= 2
    available_ranks = set(range(12)).difference([fv_code, 0])
    if ncards == 1:
        hcs1 = codes_to_mask((fv_code, RANK_CODE['2'] + 13))
        boards = list(itertools.combinations(available_ranks, 4))
        boards = [codes_to_mask(x) for x in boards]
    elif ncards == 2:
        hcs1 = codes_to_mask((fv_code, RANK_CODE['2']))
        boards = list(itertools.combinations(available_ranks, 3))
        boards = [codes_to_mask(x) for x in boards]
    all_pctiles = []
    for board in boards:
        pctile = get_sd_rank_high(hcs1, board)
        all_pctiles.append(pctile)
    return np.median(all_pctiles)

def get_sd_rank_high(hcs1_mask, board_mask):
    deck = set(range(52))
    hcs1_codes = handmask_to_codes(hcs1_mask)
    board_codes = handmask_to_codes(board_mask)
    deck = list(deck.difference(hcs1_codes).difference(board_codes))
    assert len(deck) == 52 - len(hcs1_codes) - len(board_codes)
    mask1 = hcs1_mask | board_mask
    other_masks = peval_ex.calc_permutations(deck, 2)
    other_masks = [x | board_mask for x in other_masks]
    masks = [mask1] + other_masks
    masks = np.array(masks, dtype=np.int64)
    res = np.zeros(len(masks), dtype=np.int32)
    peval_ex.evaluate_high(masks, res)
    rank1 = res[0]
    all_ranks = np.sort(res[1:])
    pctile = np.sum(rank1 >= all_ranks) / len(all_ranks)
    return pctile