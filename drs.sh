#!/bin/sh
./waf
cp build/libextensions.so .
./waf --run drs
