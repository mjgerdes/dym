#! /bin/bash
# This command will run Did You Mean to read in the brownprob corpus file(with probability annotations) and print all found corrections (better ones first) to standard output
./bin/dym -ape2 data/brownprob < data/testwords
