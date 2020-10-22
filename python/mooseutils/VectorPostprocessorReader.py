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

    MOOSE outputs VectorPostprocessor data in separate files for each timestep, using the timestep as
    a prefix. For example: file_000.csv, file_001.csv, etc.

    Therefore, a pattern acceptable for use with the python glob package must be supplied. For the
    above files, "file_*.csv" should be supplied.

    This object manages the loading and unloading of data and should always be in a valid state,
    regardless of the existence of a file. It will also append new data and remove old/deleted data
    on subsequent calls to "update()".
    """
    def __init__(self, pattern, run_start_time=0):
        self._pattern = pattern
        self._timedata = MooseDataFrame(self._pattern.replace('*', 'time'),
                                        run_start_time=None,
                                        index='timestep')
        self._frames = dict()
        self._time = -1
        self._index = None
        self._run_start_time = run_start_time
        self.update()

    @property
    def data(self):
        return self._frames.get(self._index, pandas.DataFrame())

    @property
    def filename(self):
        if self._frames:
            return self._frames[self._index].filename

    def __getitem__(self, keys):
        """
        Operator[] returns the data for the current time.

        Args:
            keys[str|list]: The key(s) to return.
        """
        return self._frames[self._index][keys]

    def __bool__(self):
        """
        Allows this object to be used in boolean cases.

        Example:
            data = VectorPostprocessorReader('files_*.csv')
            if not data:
                print 'No data found!'
        """
        return self._index in self._frames

    def __contains__(self, variable):
        """
        Returns true if the variable exists in the data structure.
        """
        return variable in self._frames[self._index]

    def times(self):
        """
        Returns the list of available time indices contained in the data.
        """
        return sorted(self._frames.keys())

    def clear(self):
        """
        Remove all data.
        """
        self._frames = dict()
        self._index = None
        self._time = None

    def variables(self):
        """
        Return a list of postprocessor variable names listed in the reader.
        """
        if self._index is not None:
            return self._frames[self._index].data.columns.tolist()

    def update(self, time=None):
        """
        Update data by adding/removing files.

        time[float]: The time at which the data should be returned.
        """

        # Update the time
        if time is not None:
            self._time = time

        # Update the time data file
        self._timedata.update()

        # The list of files from the supplied pattern
        last_modified = 0.0
        for fname in sorted(glob.glob(self._pattern)):
            if fname.endswith('LATEST') or fname.endswith('FINAL') or (fname == self._timedata.filename):
                continue
            idx = self._timeHelper(fname)

            mdf = self._frames.get(idx, None)
            if mdf is None:
                mdf = MooseDataFrame(fname, run_start_time=self._run_start_time, update=False,
                                     peacock_index=True)
                self._frames[idx] = mdf

        # Clean up old and empty data
        for idx in list(self._frames.keys()):
            mdf = self._frames[idx]
            mdf.update()
            if mdf.empty():
                self._frames.pop(idx)
            elif (mdf.modified < last_modified):
                self._frames.pop(idx)
            elif mdf.filesize == 0:
                self._frames.pop(idx)
            else:
                last_modified = mdf.modified

        self.__updateCurrentIndex()

    def repr(self):
        """
        Return components for building script.

        Returns:
           (output, imports) The necessary script and include statements to re-create data load.
        """

        imports = ['import mooseutils']
        output = ['\n# Read VectorPostprocessor Data']
        output += ['data = mooseutils.VectorPostprocessorReader({})'.format(repr(self._pattern))]
        return output, imports

    def _timeHelper(self, filename):
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

    def __updateCurrentIndex(self):
        """
        Helper for setting the current key for the supplied time.
        """
        if not self._frames:
           index = None

        # Return the latest time
        elif self._time == -1:
            index = self.times()[-1]

        # Return the specified time
        elif self._time in self._frames:
            index = self._time

        # Find nearest time
        else:
            times = self.times()
            n = len(times)
            idx = bisect.bisect_right(times, self._time) - 1
            if idx < 0:
                idx = 0
            elif idx > n:
                idx = -1
            index = times[idx]

        self._index = index
