#!/usr/bin/env python
import re

in_anythingnew = 0
in_anythingnew_p = re.compile('^2.*OnInterest.*anythingnew.*')
in_somethingnew = 0
in_somethingnew_p = re.compile('^2.*OnInterest.*somethingnew.*')
gen_msg = 0
gen_msg_p = re.compile('.*GenMessage.*')
in_data = 0
in_data_p = re.compile('.*ProcessDataData.*')




presice_time_p = re.compile('\d+\.\d{6}')
head_time_p = re.compile('[^ ]*s')
d={}
for line in open('a.txt'):
	# received anything new
	if in_anythingnew_p.match(line):
		in_anythingnew += 1
	# received something new
	if in_somethingnew_p.match(line):
		in_somethingnew += 1
	# gen msg
	if gen_msg_p.match(line):
		gen_msg += 1
		time = float(presice_time_p.findall(line)[0])
		d[time]=[]
	# received actual data
	if in_data_p.match(line):
		in_data += 1
		htime = float(head_time_p.findall(line)[0][:-1])
		ptime = float(presice_time_p.findall(line)[0])
		d[ptime].append(round(htime-ptime, 3))

print
total_sum = 0
total_len = 0
for key in sorted(d):
	if key>29: break
	total_sum += sum(d[key])
	total_len += len(d[key])
	print key, '\t', len(d[key]), '\t', sum(d[key])/len(d[key])
print 'Total Average Delay:', total_sum/total_len

print
print 'Received Anything New:', in_anythingnew
print 'Received Something New:', in_somethingnew
print 'Total Msg:', gen_msg
