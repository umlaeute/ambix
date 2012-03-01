#!/bin/sh

cd  ${0%/*}

LD_LIBRARY_PATH=/home/cubemixer/src/libsndfile-cust-chunk/src/.libs pd $@
