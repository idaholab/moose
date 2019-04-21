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
import mooseutils
import chigger
from .ChiggerFilterBase import ChiggerFilterBase
class ContourFilter(ChiggerFilterBase):
    """
    Filter for computing and visualizing contours.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerFilterBase.getOptions()
        opt.add('count', 10, "The number of contours to be automatically generated between the "
                             "specified range.", vtype=int)
        opt.add('levels', None, "Explicitly define the contour levels, if this options is "
                                "provided 'count' is ignored.", vtype=list)
        return opt

    def __init__(self, **kwargs):
        super(ContourFilter, self).__init__(vtkfilter_type=vtk.vtkContourFilter, **kwargs)

    def update(self, **kwargs):
        """
        Computes the contour levels for the vtkContourFilter.
        """
        super(ContourFilter, self).update(**kwargs)

        varinfo = self._source.getCurrentVariableInformation()
        if varinfo.object_type != chigger.exodus.ExodusReader.NODAL:
            raise mooseutils.MooseException('ContourFilter currently only works with nodal '
                                            'variables.')

        self._vtkfilter.SetInputArrayToProcess(0, 0, 0, vtk.vtkDataObject.FIELD_ASSOCIATION_POINTS,
                                               varinfo.name)

        if self.isOptionValid('levels'):
            levels = self.getOption('levels')
            n = len(levels)
            self._vtkfilter.SetNumberOfContours(n)
            for i in range(n):
                self._vtkfilter.SetValue(i, levels[i])
        elif self.isOptionValid('count'):
            rng = self._source.getVTKMapper().GetScalarRange()
            self._vtkfilter.GenerateValues(self.getOption('count'), rng)
        else:
            mooseutils.MooseException('Either the "levels" or the "count" options must be used.')
