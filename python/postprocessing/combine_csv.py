#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import argparse, sys, os, csv

parser = argparse.ArgumentParser(description="Combine series of csv outputs in separate files for each time step into a single file")
parser.add_argument("-d", "--delimiter", type = str, help="delimiter for output file")
parser.add_argument("-w", "--write_header", action="store_true", help="write header in output file")
parser.add_argument("-o", "--output", type = str, help="output file", required=True)
parser.add_argument("-x", "--x_variable", type = str, help="x variable name")
parser.add_argument("-y", "--y_variable", type = str, help="y variable name", required=True)
parser.add_argument("-l", "--last", type = int, help="take last n steps")
parser.add_argument("-s", "--start", type = int, help="start at step")
parser.add_argument("-e", "--end", type = int, help="end at step")
parser.add_argument("basename", type = str, help="Basename of csv file time series")

args=parser.parse_args()
basename = args.basename
x_varname = args.x_variable
y_varname = args.y_variable
outfilename = args.output
lastn = args.last
startt = args.start
endt = args.end
write_header = args.write_header

delimiter = args.delimiter
if delimiter == None:
    delimiter = ','

csvfile_names=[]
csvfiles=[]
csvdictreaders=[]
times=[]
time_idx=0
while(True):
    file_name = basename+"%04d" %(time_idx)+'.csv'
    time_idx += 1
    if not os.path.isfile(file_name):
        break
    csvfile_names.append(file_name)

if lastn != None:
    if startt != None or endt != None:
        sys.stderr.write("Cannot specify --last together with --start or --end\n")
        sys.exit(1)
    startt = len(csvfile_names) - lastn
    endt = len(csvfile_names) - 1
else:
    if startt == None:
        startt = 0
    if endt == None:
        endt = len(csvfile_names) - 1

for i, file_name in enumerate(csvfile_names):
    if i >= startt and i <= endt:
        csvfiles.append(open(file_name))
        csvdictreaders.append(csv.DictReader(csvfiles[-1]))
        times.append(str(i))

if len(csvfiles) == 0:
    sys.stderr.write("No files to combine\n")
    sys.exit(1)

fieldnames = []
if x_varname != None:
    fieldnames += [x_varname]
fieldnames += times

outfile = open(outfilename,'w')
csvwriter = csv.DictWriter(outfile, delimiter=delimiter, lineterminator='\n', fieldnames=fieldnames)

if write_header:
    csvwriter.writeheader()

keep_reading = True
while (keep_reading):
    for icsv, csvdictreader in enumerate(csvdictreaders):
        try:
            curr_line_data = csvdictreader.next()
        except StopIteration:
            keep_reading = False
            break
        if icsv == 0:
            line_data = {}
        if x_varname != None:
            try:
                cur_xvar = curr_line_data[x_varname]
            except KeyError:
                sys.stderr.write("Cannot find '"+x_varname+"' field in file: "+csvfile_names[icsv]+"\n")
                sys.exit(1)
            if icsv == 0:
                line_data[x_varname] = cur_xvar
            else:
                if cur_xvar != line_data[x_varname]:
                    sys.stderr.write("Inconsistent value for '"+x_varname+"' field in file: "+csvfile_names[icsv]+"\n")
                    sys.stderr.write("cur: "+cur_xvar+" orig: "+line_data[x_varname]+"\n")
                    sys.exit(1)
        try:
            cur_data = curr_line_data[y_varname]
        except KeyError:
            sys.stderr.write("Cannot find '"+y_varname+"' field in file: "+csvfile_names[i]+"\n")
            sys.exit(1)
        if icsv == 0:
            line_data[times[icsv]] = cur_data
        else:
            line_data[times[icsv]] = cur_data
    if keep_reading:
        csvwriter.writerow(line_data)

for csvfile in csvfiles:
    csvfiles[icsv].close()
outfile.close()
