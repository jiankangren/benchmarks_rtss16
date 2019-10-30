#!/usr/bin/python

import sys
import string
import os
import math
import random
import copy


# Default min and max number of segments
num_segments_min = 10
num_segments_max = 20

# Default min and max number of strands per segment
# Type 1: 100,000 - 200,000
# Type 2: 10,000 - 20,000
# Type 3: 1,000 - 2,000
num_strands_min = 1000#10000 #100000 #10000 #1000
num_strands_max = 2000#20000 #200000 #20000 #2000

# Default min and max segment length (in nanoseconds)
# Type 1: 5 - 10
# Type 2: 2,000 - 4,500
# Type 3: 20,000 - 50,000

segment_len_min = 20000 #2000 #5 #2000 #20000
segment_len_max = 50000 #4500 #10 #4500 #50000


# The number of segments is taken randomly from a specific range
def gen_num_segments():
	return random.randrange(num_segments_min, num_segments_max+1)


# Return the number of strands of a particular segment
def gen_num_strands():
	return random.randrange(num_strands_min, num_strands_max+1)


# Return the length of a particular segment
def gen_segment_length():
	return random.randrange(segment_len_min, segment_len_max+1)


def main():
	task = []

	num_segments = gen_num_segments()
	for segment in range(num_segments):
		num_strands = gen_num_strands()
		seg_len = gen_segment_length()
		seg_info = [num_strands, seg_len]
		task.append(seg_info)

	f = open("task.info", 'w')
	line = str(num_segments) + ' '
	for i in range(num_segments):
		line += str(task[i][0]) + ' ' + str(task[i][1]) + ' '
		
	f.write(line)
	return

main()
