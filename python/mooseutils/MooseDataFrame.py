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
    INVALID = 1
    OLDFILE = 2
    UPDATED = 3

    def __init__(self, filename, index=None, run_start_time=None):

        self.filename = filename
        self.modified = None
        self.data = pandas.DataFrame()
        self._index = index
        self._run_start_time = run_start_time
        self.update()

    def __getitem__(self, key):
        """
        Provides [] access to data.

        Args:
            key[str|list]: The key(s) to extract.
        """
        if self.data.empty:
            return pandas.Series()
        return self.data[key]

    def __contains__(self, key):
        """
        Test if a key is stored in the data.
        """
        return (key in self.data)

    def __nonzero__(self):
        """
        Return False if the data is empty.
        """
        return not self.data.empty

    def clear(self):
        """
        Remove existing data.
        """
        self.modified = None
        self.data = pandas.DataFrame()

    def update(self):
        """
        Update with new data.
        """
        retcode = MooseDataFrame.NOCHANGE

        file_exists = os.path.exists(self.filename)
        if file_exists and (os.path.getmtime(self.filename) < self._run_start_time):
            self.clear()
            message.mooseDebug("The csv file {} exists but is old compared to the run start time.".format(self.filename), debug=True)
            retcode = MooseDataFrame.OLDFILE

        elif not os.path.exists(self.filename):
            self.clear()
            message.mooseDebug("The csv file {} does not exist.".format(self.filename))
            retcode = MooseDataFrame.INVALID

        else:
            modified = os.path.getmtime(self.filename)
            if modified != self.modified:
                retcode = MooseDataFrame.UPDATED
                try:
                    self.modified = modified
                    self.data = pandas.read_csv(self.filename)
                    if self._index:
                        self.data.set_index(self._index, inplace=True)
                    message.mooseDebug("Reading csv file: {}".format(self.filename))
                except:
                    self.clear()
                    message.mooseDebug("Unable to read file {} it likely does not contain data.".format(self.filename))

        return retcode
