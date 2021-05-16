#!/bin/bash

cat sent1.csv | cut -d ',' -f 5 > fileS1
cat sent2.csv | cut -d ',' -f 5 > fileS2
cat received1.csv | cut -d ',' -f 5 > fileR1
cat received2.csv | cut -d ',' -f 5 > fileR2
diff -s fileS1 fileR2
diff -s fileS2 fileR1
