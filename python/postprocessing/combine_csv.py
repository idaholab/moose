#!/usr/bin/env python

import argparse, re, sys, os

parser = argparse.ArgumentParser(description="Combine series of csv outputs in separate files for each time step into a single file")
parser.add_argument("-o", "--output", type = str, help="output file", required=True)
parser.add_argument("-v", "--variable", type = str, help="variable name", required=True)
parser.add_argument("-l", "--last", type = int, help="take last n steps")
parser.add_argument("-s", "--start", type = int, help="start at step")
parser.add_argument("-e", "--end", type = int, help="end at step")
parser.add_argument("basename", type = str, help="Basename of csv file time series")

args=parser.parse_args()
basename = args.basename
varname = args.variable
outfilename = args.output
lastn = args.last
startt = args.start
endt = args.end

file_names=[]
files=[]
times=[]
time_idx=0
while(True):
  file_name = basename+"%04d" %(time_idx)+'.csv'
  time_idx += 1
  if not os.path.isfile(file_name):
    break
  file_names.append(file_name)

if lastn != None:
  if startt != None or endt != None:
    sys.stderr.write("Cannot specify --last together with --start or --end\n")
    sys.exit(1)
  startt = len(file_names) - lastn
  endt = len(file_names) - 1
else:
  if startt == None:
    startt = 0
  if endt == None:
    endt = len(file_names) - 1

for i, file_name in enumerate(file_names):
  if i >= startt and i <= endt:
    files.append(open(file_name,'r'))
    times.append(i)

outfile = open(outfilename,'w')

position_idx = 0
data_idx = 0
found_position = False
found_data = False

#read the title line
for i, tfile in enumerate(files):
  line = tfile.readline().rstrip()
  splitdata=re.split('\s*,\s*', line)
  for j, col in enumerate(splitdata):
    if (splitdata[j] == 'id'):
      position_idx = j
      found_position = True
      break
  for j, col in enumerate(splitdata):
    if (splitdata[j] == varname):
      data_idx = j
      found_data = True
      break

outfile.write("#id, ")
for i in xrange(len(files)):
  outfile.write(str(times[i]))
  if i < len(files)-1:
    outfile.write(", ")
outfile.write("\n")

if (not found_position):
  sys.stderr.write("Could not find 'id' column in csv files\n")
  sys.exit(1)

if (not found_data):
  sys.stderr.write("Could not find '"+varname+"' column in csv files\n")
  sys.exit(1)

#read the data lines
eof = False
while(eof == False):
  for i, tfile in enumerate(files):
    line = tfile.readline().rstrip()
    if line == "":
      eof = True
      break
    splitdata=re.split('\s*,\s*', line)
    if i==0:
      outfile.write(str(splitdata[position_idx])+", ")
    outfile.write(str(splitdata[data_idx]))
    if i < len(files)-1:
      outfile.write(", ")
    else:
      outfile.write("\n")
