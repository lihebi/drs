#!/bin/bash
cat ../9/aggregate-trace.txt | grep "^2[2-7].*net.*InInterest" > drs.txt
cat ../../chrono/9/aggregate-trace.txt | grep "^2[2-7].*net.*InInterest" > chrono.txt
