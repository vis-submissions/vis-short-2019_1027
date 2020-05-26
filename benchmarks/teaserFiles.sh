#!/bin/bash

../build/gd -bench -mode=rtx -n=1 -o=../results/teaser1.tlp ../data/gephi/gephi_100k.csv
../build/gd -bench -mode=rtx -n=100 -o=../results/teaser100.tlp ../data/gephi/gephi_100k.csv
../build/gd -bench -mode=rtx -n=2000 -o=../results/teaser2000.tlp ../data/gephi/gephi_100k.csv
../build/gd -bench -mode=rtx -n=12000 -o=../results/teaser12000.tlp ../data/gephi/gephi_100k.csv
