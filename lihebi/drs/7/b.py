#!/usr/bin/env python
import sys
sum=0
filename = sys.argv[1]
for line in open(filename):
	sum += int(line.split()[5])

print sum
