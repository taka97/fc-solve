#!/usr/bin/env python3
#
# find-freecell-deal-index.py - a program to find out the Freecell deal index
# based on the initial cards layout.
#
# Copyright by Shlomi Fish, 2018
#
# Licensed under the MIT/Expat License.

# imports
import sys
import re
from find_index_lib_py import ffi, lib

if sys.version_info > (3,):
    long = int
    xrange = range


class Card(object):
    def __init__(self, id, rank, suit, print_ts):
        self.id, self.rank, self.suit, self.print_ts = id, rank, suit, print_ts

    def rank_s(self):
        ret = "0A23456789TJQK"[self.rank]
        if ((not self.print_ts) and ret == 'T'):
            ret = '10'
        return ret

    def suit_s(self):
        return 'CSHD'[self.suit]

    def to_s(self):
        ret = self.rank_s() + self.suit_s()
        return ret


def createCards(num_decks, print_ts):
    id = 0
    ret = []
    for d in range(num_decks):
        for s in range(4):
            for r in range(13):
                ret.append(Card(id, r+1, s, print_ts))
                id += 1
    return ret


def shlomif_main(args):
    output_to_stdout = True
    is_ms = False
    while args[1][0] == '-':
        if (args[1] == "-o"):
            args.pop(0)
            if not len(args):
                raise ValueError("-o must accept an argument.")
            output_to_stdout = False
            args.pop(0)
        elif (args[1] == '--ms'):
            args.pop(0)
            is_ms = True
        elif (args[1] == '-'):
            break
        else:
            raise ValueError("Unknown flag " + args[1] + "!")

    if not is_ms:
        raise ValueError("only --ms is supported for now!")
    input_from_stdin = True
    input_fn = None
    if (len(args) >= 2):
        if (args[1] != "-"):
            input_fn = args[1]
            input_from_stdin = False
            args.pop(0)

    content = []
    if input_from_stdin:
        content = sys.stdin.readlines()
    else:
        with open(input_fn) as f:
            content = f.readlines()
    content = ''.join(content)

    rank_s = 'A23456789TJQK'
    rank_re = r'[' + rank_s + r']'
    suit_s = 'CSHD'
    suit_re = r'[' + suit_s + r']'

    card_re = rank_re + suit_re
    card_re_paren = r'(' + card_re + r')'

    def make_line(n):
        return r':?[ \t]*' + card_re_paren + \
            (r'[ \t]+' + card_re_paren) * (n-1) + r'[ \t]*\n'

    complete_re = r'^' + make_line(7) * 4 + make_line(6) * 4 + '\s*$'

    m = re.match(complete_re, content)
    if not m:
        raise ValueError("Could not match.")

    cards = createCards(1, True)
    c = []
    for i in range(13):
        for j in (0, 39, 26, 13):
            c.append(cards[i + j])
    cards = [x.to_s() for x in c]

    # Reverse shuffle:
    ints = []
    n = 4 * 13 - 1
    for i in range(n):
        col = i // 8
        row = i % 8
        s = m.group(1 + col + (4*7+(row-4)*6 if row >= 4 else row*7))
        idx = [j for j in range(n+1) if cards[j] == s]
        if len(idx) != 1:
            raise ValueError("Foo")
        j = idx[0]
        ints.append(j)
        cards[j] = cards[n]
        n -= 1

    obj = lib.fc_solve_user__find_deal__alloc()
    lib.fc_solve_user__find_deal__fill(
        obj, "".join(["%-10d" % x for x in ints]))
    ret = int(ffi.string(lib.fc_solve_user__find_deal__run(
        obj, "1", "%d" % ((1 << 33) - 1))))

    ret_code = 0
    if ret >= 0:
        if output_to_stdout:
            print("Found deal = %d" % ret)
        ret_code = 0
    else:
        print("Not found!")
        ret_code = -1
    lib.fc_solve_user__find_deal__free(obj)
    return ret_code


if __name__ == "__main__":
    sys.exit(shlomif_main(sys.argv))
