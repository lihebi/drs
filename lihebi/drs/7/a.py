#!/usr/bin/env python
ll=[]
d={}
for line in open('tmp.txt'):
	l = line.split(' ')
	if d.has_key(l[1]):
		ll.append(round(float(l[0][:-1])-d[l[1]], 3))
	else:
		d[l[1]]=float(l[0][:-1])

print ll
	
