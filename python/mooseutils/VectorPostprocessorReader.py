#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import glob
import pandas
import bisect

from .MooseDataFrame import MooseDataFrame
from . import message

class VectorPostprocessorReader(object):
    """
    A Reader for MOOSE VectorPostprocessor data.

    Args:
       pattern[str]: A pattern of files (for use with glob) for loading.

    MOOSE outputs VectorPostprocessor data in separate files for each timestep, using the timestep as a prefix. For
    example: file_000.csv, file_001.csv, etc.

    Therefore, a pattern acceptable for use with the python glob package must be supplied. For the above files,
    "file_*.csv" should be supplied.

    This object manages the loading and unloading of data and should always be in a valid state, regardless of the
    existence of a file. It will also append new data and remove old/deleted data on subsequent calls to "update()".
    """

    #: Status flags for loading/reloading/removing csv files (see "_modified").
    NO_CHANGE = 0
    NEW_DATA = 1
    OLD_DATA = 2

    def __init__(self, pattern, run_start_time=None):

        self.filename = pattern
        self._timedata = MooseDataFrame(self.filename.replace('*', 'time'), run_start_time=None, index='timestep')

        self._modified_times = dict()
        #self._run_start_time = run_start_time
        self.data = pandas.Panel()
        self.update()

        self._minimum_modified = 0.0#self._run_start_time if self._run_start_time else 0.0


    def __call__(self, keys, time=None, exact=False, **kwargs):
        """
        Operator() returns the latest time or the desired time.

        Args:
            keys[str|list]: The key(s) to return.
            time[float]: The time at which the data should be returned.
            exact[bool]: When the time supplied is not an exact match, if 'exact=False' is provided the nearest time
                          less than the provided time is returned, when false an empty DataFrame is returned.
        """

        # Return the latest time
        if time == None:
            return self.data.iloc[-1][keys]

        # Return the specified time
        elif time in self.data.keys().values:
            return self.data[time][keys]

        # Time not found and 'exact=True'
        elif exact:
           return pandas.DataFrame()

        # Time not found and 'exact=False'
        else:
            times = self.data.keys()
            n = len(times)
            idx = bisect.bisect_right(times, time) - 1
            if idx < 0:
                idx = 0
            elif idx > n:
                idx = -1
            return self.data.iloc[idx][keys]


    def __getitem__(self, key):
        """
        Column based access to VectorPostprocessor data.

        Args:
            key[str]: A VectorPostprocessor name.

        Returns:
            pandas.DataFrame containing the data for all available times (column).

        """
        if self.data.empty:
            return pandas.DataFrame()
        else:
            return self.data.minor_xs(key)

    def __nonzero__(self):
        """
        Allows this object to be used in boolean cases.

        Example:
            data = VectorPostprocessorReader('files_*.csv')
            if not data:
                print 'No data found!'
        """
        return not self.data.empty

    def __contains__(self, variable):
        """
        Returns true if the variable exists in the data structure.
        """
        return variable in self.variables()

    def times(self):
        """
        Returns the list of available time indices contained in the data.
        """
        return self.data.keys().values.tolist()

    def clear(self):
        """
        Remove all data.
        """
        self.data = pandas.Panel()
        self._modified_times = dict()
        self._minimum_modified = 0.0# self._run_start_time if self._run_start_time else 0.0

    def variables(self):
        """
        Return a list of postprocessor variable names listed in the reader.
        """
        return self.data.axes[2]

    def update(self):
        """
        Update data by adding/removing files.
        """

        # Return code (1 = something changed)
        retcode = 0

        # Update the time data file
        self._timedata.update()

        # The current filenames, time index, and modified status
        filenames, indices, modified = self._filenames()

        # Clear the data if empty
        if not filenames:
            self.clear()
            return 1

        # Loop through the filenames
        for fname, index, mod in zip(filenames, indices, modified):

            if mod == VectorPostprocessorReader.NEW_DATA:

                try:
                    df = pandas.read_csv(fname)
                except:
                    message.mooseWarning('The file {} failed to load, it is likely empty.'.format(fname))
                    continue

                df.insert(0, 'index (Peacock)', pandas.Series(df.index, index=df.index))
                if self.data.empty:
                    self.data = pandas.Panel({index:df})
                else:
                    self.data[index] = df
                retcode = 1

            elif (mod == VectorPostprocessorReader.OLD_DATA) and (index in self.data.keys()):
                self.data.pop(index)
                retcode = 1

        # Remove missing files
        for key in self.data.keys():
            if key not in indices:
                self.data.pop(key)
                retcode = 1

        return retcode

    def repr(self):
        """
        Return components for building script.

        Returns:
           (output, imports) The necessary script and include statements to re-create data load.
        """

        imports = ['import mooseutils']
        output = ['\n# Read VectorPostprocessor Data']
        output += ['data = mooseutils.VectorPostprocessorReader({})'.format(repr(self.filename))]
        return output, imports


    def _filenames(self):
        """
        Returns the available filenames, time index, and modified status. (protected)
        """

        # The list of files from the supplied pattern
        filenames = []
        for fname in sorted(glob.glob(self.filename)):
            if fname.endswith('LATEST') or fname.endswith('FINAL'):
                continue
            filenames.append(fname)

        # Remove the "_time.csv" from the list, if it exists
        try:
            filenames.remove(self._timedata.filename)
        except:
            pass

        # Update the minimum modified time
        if len(filenames) > 0:
            self._minimum_modified = os.path.getmtime(filenames[0])
        else:
            self._minimum_modified = 0

        # Determine the time index and modified status
        indices, modified = [], []
        for fname in filenames:
            indices.append(self._time(fname))
            modified.append(self._modified(fname))

        return filenames, indices, modified


    def _modified(self, filename):
        """
        Determine the modified status of a filename. (protected)
        """

        modified = os.path.getmtime(filename)
        if modified < self._minimum_modified:
            self._modified_times.pop(filename, None)
            return VectorPostprocessorReader.OLD_DATA

        elif (filename not in self._modified_times) or (modified > self._modified_times[filename]):
            self._modified_times[filename] = os.path.getmtime(filename)
            return VectorPostprocessorReader.NEW_DATA

        return VectorPostprocessorReader.NO_CHANGE

    def _time(self, filename):
        """
        Determine the time index. (protected)
        """

        idx = filename.rfind('_') + 1
        tstep = int(filename[idx:-4])
        if not self._timedata:
            return tstep
        else:
            try:
                return self._timedata['time'].loc[tstep]
            except Exception:
                return tstep
