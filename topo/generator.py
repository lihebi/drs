#!/usr/bin/env python

print 'router'
x=0
y=1
linkpara = '\t10Mbps\t1\t10ms\t10'
for x in [0,1,2,3,4]:
	for y in [0,1,2,3,4]:
		print 'Node'+str(x)+str(y)+'\tNA\t'+str(x)+'\t'+str(y)

print 'link'
for x in [0,1,2,3,4]:
	for y in [0,1,2,3,4]:
		if x==4 and y==4: continue
		print 'Node'+str(x)+str(y),
		if x==4:
			print '\tNode'+str(x)+str(y+1),
		elif y==4:
			print '\tNode'+str(x+1)+str(y),
		else:
			print '\tNode'+str(x+1)+str(y),
			print linkpara
			print 'Node'+str(x)+str(y),
			print '\tNode'+str(x)+str(y+1),
		print linkpara
