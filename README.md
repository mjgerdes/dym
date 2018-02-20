#DYM

Did You Mean - Fast, Tri based misspelled word correction.

dym is a program that will, based on a supplied dictionary file, attempt to give correction suggestions to words supplied via standard input.
The dictionary files can, if desired, be supplemented with probabilities for words.
Internally, dym does a beam search on a tri to find words that are within a specified edit-distance of the input word, which should be fast.

#Formats

Input corpora are one word per line. Probability annotated corpora are one word, the tab character, and then the probability of the word.
Examples are provided in data/ .

# Usage

Usage: ./dym [-abps] CORPUSFILE
Reads words from standard input and prints suggestions to standard output.
Examples
  Print all found suggestions, using probability based corpus and maximum edit distance 2:
./dym -apd2 corpus.txt
Options:
 -a, all - Return all found suggestions (default).
 -b, best - Return only the single best solution.
 -p, probability - Use tab seperated word corpus with floating point numbers to indicate word probability (default).
 -s, simple - Use a simple, non-probability corpus.
 -eN edit distance 0 <= N <= 9, Find suggestions with a maximum of N unit edit operations (default 1).