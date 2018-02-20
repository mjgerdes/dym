#! /usr/bin/python

from nltk.corpus import brown

ws = brown.words()

for w in ws:
  if(str(w).isalpha()):
    print(w)

