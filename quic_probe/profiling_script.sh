#!/usr/bin/env bash
# If gprof is not install run apt-get install binutils
# Read result in analysis.txt
rm -rf ./profile_test/
mkdir profile_test
cd profile_test
cmake -DCMAKE_C_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg quic_client ../
make
./quic_probe
gprof quic_probe > analysis.txt
