#!/bin/python

import sys

import statistics

file_name = sys.argv[1]
f = open(file_name)
d = f.readlines()
f.close()

d = [float(x.strip()) for x in d]

from matplotlib import pyplot

print("mean: ", statistics.mean(d))
print("median: ", statistics.median(d))

pyplot.plot(d)
pyplot.show()
