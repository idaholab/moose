#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import glob
import mooseutils
from ExodusReader import ExodusReader
from .. import base

class MultiAppExodusReader(base.ChiggerObject):
    """
    A reader for MultiApp Exodus files.

    This class is simply a wrapper that creates and ExodusReader object for each file found using
    glob from the supplied pattern.

    Inputs:
        pattern[str]: A string containing a glob pattern for MultiApp ExodusII output files from
                      MOOSE.
    """

    @staticmethod
    def validOptions():
        opt = ExodusReader.validOptions()
        return opt

    def __init__(self, pattern, **kwargs):
        super(MultiAppExodusReader, self).__init__(**kwargs)

        self.__readers = []
        for filename in glob.glob(pattern):
            self.__readers.append(ExodusReader(filename, **kwargs))

    def __iter__(self):
        """
        Provide iterator access to the readers.
        """
        for reader in self.__readers:
            yield reader

    def __getitem__(self, index):
        """
        Provide operator[] access to the readers.
        """
        return self.__readers[index]

    def __str__(self):
        """
        Return the ExodusReader information for each multiapp file.
        """
        out = ''
        for reader in self.__readers:
            out += '\n\n' + mooseutils.colorText(reader.filename(), 'MAGENTA')
            out += str(reader)
        return out

    def setOption(self, *args, **kwargs):
        """
        Set single option for all contained readers.
        """
        super(MultiAppExodusReader, self).setOption(*args, **kwargs)
        for reader in self.__readers:
            reader.setOption(*args, **kwargs)

    def setOptions(self, *args, **kwargs):
        """
        Set options for all contained readers.
        """
        super(MultiAppExodusReader, self).setOptions(*args, **kwargs)
        for reader in self.__readers:
            reader.setOptions(*args, **kwargs)

    def update(self, *args, **kwargs):
        """
        Update all readers.
        """
        super(MultiAppExodusReader, self).update(*args, **kwargs)
        for reader in self.__readers:
            reader.update(*args, **kwargs)
