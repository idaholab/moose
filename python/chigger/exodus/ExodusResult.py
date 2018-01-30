#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from ExodusSource import ExodusSource
from ExodusReader import ExodusReader
from MultiAppExodusReader import MultiAppExodusReader
import mooseutils
from .. import base

class ExodusResult(base.ChiggerResult):
    """
    Result object to displaying ExodusII data from a single reader.
    """
    @staticmethod
    def getOptions():
        opt = base.ChiggerResult.getOptions()
        opt += ExodusSource.getOptions()
        return opt

    def __init__(self, reader, **kwargs):

        # Build the ExodusSource objects
        if isinstance(reader, ExodusReader):
            sources = [ExodusSource(reader)]
        elif isinstance(reader, MultiAppExodusReader):
            sources = [ExodusSource(r) for r in reader]
        else:
            raise mooseutils.MooseException('The supplied object of type {} is not supported, '
                                            'only ExodusReader and MultiAppExodusReader objects '
                                            'may be utilized.'.format(reader.__class__.__name__))

        # Supply the sources to the base class
        super(ExodusResult, self).__init__(*sources, **kwargs)
