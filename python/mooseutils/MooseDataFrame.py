#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import pandas

from . import message

class MooseDataFrame(object):
    """
    A wrapper for handling data from a single csv file.

    This utilizes a pandas.DataFrame for storing and accessing CSV data, while
    allowing for the file to exist/not-exist.
    """
    NOCHANGE = 0
    UPDATED = 1
    INVALID = 2
    OLDFILE = 3

    def __init__(self, filename, index=None, run_start_time=None, update=True, peacock_index=False):
        self._filename = filename
        self._data = pandas.DataFrame()
        self._modified = None
        self._index = index
        self._add_peacock_index = peacock_index
        self._run_start_time = run_start_time
        if update:
            self.update()

    @property
    def modified(self):
        if self._modified is None:
            return os.path.getmtime(self._filename)
        return self._modified

    @property
    def exists(self):
        return os.path.exists(self._filename)

    @property
    def filesize(self):
        return os.path.getsize(self._filename)

    @property
    def data(self):
        return self._data

    @property
    def filename(self):
        return self._filename

    def __getitem__(self, key):
        """
        Provides [] access to data.

        Args:
            key[str|list]: The key(s) to extract.
        """
        if self._data.empty:
            return pandas.Series()
        return self._data[key]

    def __contains__(self, key):
        """
        Test if a key is stored in the data.
        """
        return (key in self.data)

    def __bool__(self):
        """
        Return False if the data is empty.
        """
        return not self._data.empty

    def empty(self):
        """
        Return True if the data is empty.
        """
        return self._data.empty

    def clear(self):
        """
        Remove existing data.
        """
        self._modified = None
        self._data = pandas.DataFrame()

    def update(self):
        """
        Update with new data.
        """
        retcode = MooseDataFrame.NOCHANGE

        file_exists = self.exists
        if file_exists and (self._run_start_time is not None) and (os.path.getmtime(self._filename) < self._run_start_time):
            self.clear()
            message.mooseDebug("The csv file {} exists but is old ({}) compared to the run start time ({}).".format(self.filename, os.path.getmtime(self._filename), self._run_start_time), debug=True)
            retcode = MooseDataFrame.OLDFILE

        elif not file_exists:
            self.clear()
            message.mooseDebug("The csv file {} does not exist.".format(self._filename))
            retcode = MooseDataFrame.INVALID

        else:
            modified = os.path.getmtime(self._filename)
            if modified != self._modified:
                retcode = MooseDataFrame.UPDATED
                try:
                    self._modified = modified
                    self._data = pandas.read_csv(self._filename)
                    if self._index:
                        self._data.set_index(self._index, inplace=True)

                    if self._add_peacock_index:
                        self._data.insert(0, 'index (Peacock)',
                                          pandas.Series(self._data.index, index=self._data.index))
                    message.mooseDebug("Reading csv file: {}".format(self._filename))
                except:
                    self.clear()
                    message.mooseDebug("Unable to read file {} it likely does not contain data.".format(self._filename))

        return retcode
