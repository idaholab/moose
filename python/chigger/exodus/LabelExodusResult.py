#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import vtk
from .LabelExodusSource import LabelExodusSource
from .. import base

class LabelExodusResult(base.ChiggerResult):
    """
    Object for attaching labels to ExodusResult options.

    Args:
        result[ExodusResult]: The result object to label.
    """
    @staticmethod
    def getOptions():
        opt = base.ChiggerResult.getOptions()
        opt += LabelExodusSource.getOptions()
        return opt

    def __init__(self, exodus_result, **kwargs):

        # Populate the sources to display
        self._exodus_result = exodus_result
        sources = []
        for src in self._exodus_result:
            sources.append(LabelExodusSource(src, **kwargs))

        super(LabelExodusResult, self).__init__(*sources, renderer=exodus_result.getVTKRenderer(),
                                                viewport=exodus_result.getOption('viewport'),
                                                vtkmapper_type=vtk.vtkLabeledDataMapper, **kwargs)
