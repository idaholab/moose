#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .ExodusSourceLineSampler import ExodusSourceLineSampler
from ..base import ChiggerResult

class ExodusResultLineSampler(ChiggerResult):
    """
    Object for sampling ExodusSource object contained in an ExodusResult.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerResult.getOptions()
        return opt

    def __init__(self, exodus_result, **kwargs):

        self._exodus_result = exodus_result
        sources = []
        for src in self._exodus_result:
            sources.append(ExodusSourceLineSampler(src, **kwargs))

        super(ExodusResultLineSampler, self).__init__(*sources,
                                                      renderer=exodus_result.getVTKRenderer(),
                                                      viewport=exodus_result.getOption('viewport'),
                                                      **kwargs)
