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
import numpy as np
from ExodusSource import ExodusSource
from moosetools import mooseutils
from .. import geometric

class ExodusSourceLineSampler(geometric.LineSource):
    """
    Class for sampling an ExodusSource along a line.
    """

    @staticmethod
    def validParams():
        opt = geometric.LineSource.validParams()
        opt.setValue('point1', None)
        opt.setValue('point2', None)
        return opt

    def __init__(self, exodus_source, **kwargs):
        super(ExodusSourceLineSampler, self).__init__(**kwargs)

        if not isinstance(exodus_source, ExodusSource):
            msg = 'The supplied object of type {} must be a ExodusSource object.'
            raise mooseutils.MooseException(msg.format(exodus_source.__class__.__name__))

        self._distance = []
        self._exodus_source = exodus_source

        self._probe = vtk.vtkCompositeDataProbeFilter()

    def update(self, **kwargs):
        """
        Update the probe to extract the desired data.
        """
        super(ExodusSourceLineSampler, self).update(**kwargs)

        # Set the default points to the corners of the bounding box
        bnds = self._exodus_source.getBounds()
        if self.getParam('point1') is None:
            self._vtksource.SetPoint1((bnds[0], bnds[2], bnds[4]))
        if self.getParam('point2') is None:
            self._vtksource.SetPoint2((bnds[1], bnds[3], bnds[5]))

        # Compute the distance
        n = self.getParam('resolution')
        self._vtksource.Update()
        p0 = self._vtksource.GetPoint1()
        p1 = self._vtksource.GetPoint2()
        dist = np.linalg.norm(np.array(p1) - np.array(p0))
        self._distance = list(np.linspace(0, dist, num=n))

        self._probe.PassCellArraysOn()
        self._probe.SetInputConnection(self.getVTKSource().GetOutputPort())
        f = self._exodus_source.getFilters()[-1].getVTKFilter().GetOutputPort()
        self._probe.SetSourceConnection(f)
        self._probe.Update()


    def getDistance(self):
        """
        Return the distance array.
        """
        self.update()
        return self._distance

    def getSample(self, variable):
        """
        Return the sampled data for the given variable.
        """
        self.update()

        # Return data
        if self._probe.GetOutput().GetPointData().HasArray(variable):
            y = self._probe.GetOutput().GetPointData().GetArray(variable)
        else:
            mooseutils.mooseError('Unable to locate the variable, ' + variable + \
                                  ', in the supplied source data.')
            return []
        return [y.GetValue(i) for i in range(y.GetNumberOfTuples() - 1)]
