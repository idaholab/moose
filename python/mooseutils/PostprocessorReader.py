#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .MooseDataFrame import MooseDataFrame
from . import message

class PostprocessorReader(MooseDataFrame):
    """
    An extension to the MooseDataFrame to provide functionality mirroring that of the VectorPostprocessorReader.

    Args:
        filename[str]: The csv file to read.
    """

    def __init__(self, filename, **kwargs):
        super(PostprocessorReader, self).__init__(filename, **kwargs)

    def __getitem__(self, keys):
        """
        Return the data for the supplied keys.

        Args:
            keys[list]: Then names of the postprocessors to return.
            time: Required for consistent calls in Peacock, but not used in general.
            warning: When true (default) an error is produced if the users tries to use 'time' option, which does nothing.
        """
        return self._data[keys]

    def __contains__(self, variable):
        """
        Returns true if the variable exists in the data structure.
        """
        return variable in self.variables()

    def variables(self):
        return self._data.keys()

    def repr(self):
        """
        Return components for building script.

        Returns:
           (output, imports) The necessary script and include statements to re-create data load.
        """

        imports = ['import mooseutils']
        output = ['\n# Read Postprocessor Data']
        output += ['data = mooseutils.PostprocessorReader({})'.format(repr(self.filename))]
        return output, imports
