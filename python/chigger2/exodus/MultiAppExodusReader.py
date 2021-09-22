#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import glob
from moosetools import mooseutils
from .ExodusReader import ExodusReader
from .. import base

class MultiExodusReader(base.ChiggerObject):
    """
    A reader for handling multiple ExodusII files.

    This class is simply a wrapper that creates and ExodusReader object for each file found using
    glob from the supplied pattern.
    """

    # Used by utils.get_current_exodus_multi_reader for automatically adding reader objects to the
    # current ExodusSource.
    __CHIGGER_CURRENT__ = None

    @staticmethod
    def validParams():
        opt = ExodusReader.validParams()
        opt.remove('filename')
        opt.add('pattern', vtype=str, doc="The filename pattern to use for loading multiple ExodusII files.")
        return opt

    def __init__(self, **kwargs):
        MultiExodusReader.__CHIGGER_CURRENT__ = self
        base.ChiggerObject.__init__(self, **kwargs)

        self.__readers = list()
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

    def setParam(self, *args, **kwargs):
        """
        Set single option for all contained readers.
        """
        super(MultiAppExodusReader, self).setParam(*args, **kwargs)
        for reader in self.__readers:
            reader.setParam(*args, **kwargs)

    def setParams(self, *args, **kwargs):
        """
        Set options for all contained readers.
        """
        super(MultiAppExodusReader, self).setParams(*args, **kwargs)
        for reader in self.__readers:
            reader.setParams(*args, **kwargs)

    def update(self, *args, **kwargs):
        """
        Update all readers.
        """
        super(MultiAppExodusReader, self).update(*args, **kwargs)
        for reader in self.__readers:
            reader.update(*args, **kwargs)
