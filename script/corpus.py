#! /usr/bin/python

import sys
from math import log
from collections import Counter

def main(argv):
    filename = argv[1]
    d = Counter()
    n = 0.0
    for w in open(filename, "r").read().split("\n"):
        if(w.isalpha()):
            d[w] += 1
            n += 1.0

    for (word, count) in d.items():
        print word + "	" + str(log(count/n))

if(__name__=="__main__"):
    main(sys.argv)
