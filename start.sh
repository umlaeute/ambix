#!/bin/sh

cd  ${0%/*}


LD_LIBRARY_PATH=/home/cubemixer/src/libsndfile-cust-chunk/src/.libs pd -lib iemmatrix -lib iemlib1 -lib iemlib2 -lib zexy -path ${0%/*} $@
