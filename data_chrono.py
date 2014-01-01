#!/usr/bin/env python
import re

in_sync_and_recovery = 0
in_sync_and_recovery_p = re.compile('^2.*OnInterest.*broadcast.*')
in_recovery = 0
in_recovery_p = re.compile('^2.*OnInterest.*broadcast.*recovery.*')
out_recovery = 0
out_recovery_p = re.compile('^2.*SendInterest.*broadcast.*recovery.*')
gen_msg_p = re.compile('.*GenMessage.*')
in_data_p = re.compile('.*ProcessDataData.*')
presice_time_p = re.compile('\d+\.\d{6}')
head_time_p = re.compile('[^ ]*s')
d={}
for line in open('a.txt'):
	if in_sync_and_recovery_p.match(line):
		in_sync_and_recovery += 1
	if out_recovery_p.match(line):
		out_recovery += 1
	if in_recovery_p.match(line):
		in_recovery += 1
	if gen_msg_p.match(line):
		if presice_time_p.findall(line):
			time = float(presice_time_p.findall(line)[0])
			d[time] = []
		print line,
	if in_data_p.match(line):
		htime = float(head_time_p.findall(line)[0][:-1])
		ptime = float(presice_time_p.findall(line)[0])
		d[ptime].append(round(htime-ptime, 3))

print
print 'Messages and its time:'
total_sum=0
total_len=0
for key in sorted(d):
	if key>27: break
	total_sum += sum(d[key])
	total_len += len(d[key])
	print key, len(d[key]), sum(d[key])/len(d[key])
print 'Total Average Delay:', total_sum/total_len




print
print 'Total Received Sync and Recovery Interest: ', in_sync_and_recovery
print 'Total Recieved Recovery Interest: ', in_recovery
print 'Total Sent Recovery Interest: ', out_recovery
