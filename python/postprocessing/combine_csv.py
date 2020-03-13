#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, csv
import pandas

class CombineCSV(object):
    """
    Combine series of csv outputs in separate files for each time step into a single file.

    The class may be run from the command line. Command line help is available
    with '-h'.

    Args:
      basename[str]: Basename of csv time series.
      outfilename[str]: Output file path/name.
      y_varname[str]: "y" variable name.

    Kwargs:
      delimiter[str]: Separating character(s) between values, usually ",".
      write_header[bool]: True writes header to ouput file.
      x_variable[str]: "x" variable name.
      last[int]: Take only the final "last" number of steps.
      start[int]: Start at step number "start".
      end[int]: End at step number "end".
      timefile[bool]: True takes time from "*time.csv" file.
      bilinear[bool]: True formats output as bilinear file
    """

    def __init__(self, basename, outfilename, y_varname, **kwargs):
        """
        Set internal variables and initialize with data.
        """

        # Set object characteristics
        self.__ended = False
        self.__basename = basename
        self.__outfilename = outfilename
        self.__y_varname = y_varname
        self.__delimiter = kwargs.pop('delimiter', ',')
        self.__write_header = kwargs.pop('write_header', False)
        self.__x_varname = kwargs.pop('x_varname', None)
        self.__lastn = kwargs.pop('lastn', None)
        self.__startt = kwargs.pop('startt', 0)
        self.__endt = kwargs.pop('endt', None)
        self.__timefile = kwargs.pop('timefile', False)
        self.__bilinear = kwargs.pop('bilinear', False)

        # Start work
        csvfile_names = []
        csvfiles = []
        csvdictreaders = []
        times = []

        time_idx = 0
        while(True):
            file_name = self.__basename + "{0:04d}".format(time_idx) + '.csv'
            time_idx += 1
            if not os.path.isfile(file_name):
                break
            csvfile_names.append(file_name)

        if len(csvfile_names) == 0:
            raise CombineCSV.CombineError("BasenameError",
                    "Could not find any input files with basename: {0}".format(
                        self.__basename))

        # Determine start and stop
        if self.__lastn != None:
            if self.__startt != 0 or self.__endt != None:
                raise CombineCSV.CombineError("StepBoundsError",
                        "Cannot specify --last together with --start or --end")
            self.__startt = len(csvfile_names) - self.__lastn
        if self.__endt == None:
            self.__endt = len(csvfile_names) - 1

        if self.__timefile:
            df_time = pandas.read_csv(self.__basename+'time.csv')
        for i, file_name in enumerate(csvfile_names):
            if i >= self.__startt and i <= self.__endt:
                csvfiles.append(open(file_name))
                csvdictreaders.append(csv.DictReader(csvfiles[-1]))
                if self.__timefile:
                    # Swap timestep for time
                    times.append(str(df_time.iloc[i,0]))
                else:
                    times.append(str(i))

        fieldnames = []
        if self.__x_varname != None:
            fieldnames += [self.__x_varname]
        fieldnames += times

        outfile = open(self.__outfilename, 'w')
        csvwriter = csv.DictWriter(outfile, delimiter=self.__delimiter,
                lineterminator='\n', fieldnames=fieldnames)

        if self.__write_header:
            csvwriter.writeheader()

        keep_reading = True
        while (keep_reading):
            for icsv, csvdictreader in enumerate(csvdictreaders):
                try:
                    curr_line_data = csvdictreader.__next__()
                except StopIteration:
                    keep_reading = False
                    break
                if icsv == 0:
                    line_data = {}
                if self.__x_varname != None:
                    try:
                        cur_xvar = curr_line_data[self.__x_varname]
                    except KeyError as kerr:
                        for csvfile in csvfiles:
                            csvfile.close()
                        outfile.close()
                        raise CombineCSV.CombineError("XVariableError",
                                "Cannot find '" + self.__x_varname +
                                "' field in file: " +
                                csvfile_names[icsv]) from kerr
                    if icsv == 0:
                        line_data[self.__x_varname] = cur_xvar
                    else:
                        if cur_xvar != line_data[self.__x_varname]:
                            for csvfile in csvfiles:
                                csvfile.close()
                            outfile.close()
                            raise CombineCSV.CombineError("InconsistentError",
                                    "Inconsistent value for '" +
                                    self.__x_varname + "' field in file: " +
                                    csvfile_names[icsv] + "\ncur: " +
                                    cur_xvar + " orig: " +
                                    line_data[self.__x_varname])
                try:
                    cur_data = curr_line_data[self.__y_varname]
                except KeyError as kerr:
                    for csvfile in csvfiles:
                        csvfile.close()
                    outfile.close()
                    raise CombineCSV.CombineError("YVariableError",
                            "Cannot find '" + self.__y_varname +
                            "' field in file: " + csvfile_names[i]) from kerr
                line_data[times[icsv]] = cur_data
            if keep_reading:
                csvwriter.writerow(line_data)

        for csvfile in csvfiles:
            csvfile.close()
        outfile.close()

        if self.__bilinear:
            df_csv = pandas.read_csv(self.__outfilename, index_col=0)
            df_tran = df_csv.T
            all_col = df_tran.columns.map(str)
            with open(self.__outfilename, 'w') as f:
                f.write(','.join(all_col.values.tolist()) + '\n')
                df_tran.to_csv(f, header=False)

        # Final
        self._final_df = pandas.read_csv(self.__outfilename)
        self._ended = True

    class CombineError(Exception):
        """
        Class to handle all generated errors by combine_csv.

        Args:
          name[str]: A short identifier key of error.
          msg[str]: A detailed error message for output.
        """

        def __init__(self, name, msg):
            """
            Error characteristics initialized.
            """
            self._name = name
            self._msg = msg

# This file is executable and allows for running combine_csv via command line.
if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description="Combine series of csv outputs in separate files for each time step into a single file")
    parser.add_argument("basename", type=str, help="Basename of csv file time series")
    parser.add_argument("-o", "--output", type=str, help="output file", required=True)
    parser.add_argument("-d", "--delimiter", type=str, default=',', help="delimiter for output file")
    parser.add_argument("-w", "--write_header", action="store_true", help="write header in output file")
    parser.add_argument("-x", "--x_variable", type=str, help="x variable name")
    parser.add_argument("-y", "--y_variable", type=str, help="y variable name", required=True)
    parser.add_argument("-l", "--last", type=int, help="take last n steps")
    parser.add_argument("-s", "--start", type=int, help="start at step", default=0)
    parser.add_argument("-e", "--end", type=int, help="end at step")
    parser.add_argument("-t", "--timefile", action="store_true", help="time will be taken from '*time.csv' file")
    parser.add_argument("-b", "--bilinear", action="store_true", help="create piecwise bilinear file, usually requires -t and -w")
    args=parser.parse_args()

    run_program = CombineCSV(args.basename, args.output, args.y_variable,
            delimiter=args.delimiter, write_header=args.write_header,
            x_varname=args.x_variable, lastn=args.last, startt=args.start,
            endt=args.end, timefile=args.timefile, bilinear=args.bilinear)
