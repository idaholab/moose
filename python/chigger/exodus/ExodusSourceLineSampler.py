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
import mooseutils
from .ExodusSource import ExodusSource
from .. import geometric

class ExodusSourceLineSampler(geometric.LineSource):
    """
    Class for sampling an ExodusSource along a line.
    """

    @staticmethod
    def getOptions():
        opt = geometric.LineSource.getOptions()
        opt.setDefault('point1', None)
        opt.setDefault('point2', None)
        return opt

    def __init__(self, exodus_source, **kwargs):
        super(ExodusSourceLineSampler, self).__init__(**kwargs)

        if not isinstance(exodus_source, ExodusSource):
            msg = 'The supplied object of type {} must be a ExodusSource object.'
            raise mooseutils.MooseException(msg.format(exodus_source.__class__.__name__))

        self._distance = []
        self._exodus_source = exodus_source

        self._probe = vtk.vtkCompositeDataProbeFilter()

    def setOptions(self, *args, **kwargs):
        """
        Sets point1/point2 defaults.
        """
        super(ExodusSourceLineSampler, self).setOptions(*args, **kwargs)

        bounds = self._exodus_source.getBounds()
        if not self.isOptionValid('point1'):
            self.setOption('point1', bounds[0])
        if not self.isOptionValid('point2'):
            self.setOption('point2', bounds[1])

    def update(self, **kwargs):
        """
        Update the probe to extract the desired data.
        """
        super(ExodusSourceLineSampler, self).update(**kwargs)

        self._exodus_source.checkUpdateState()

        if self.isOptionValid('resolution'):
            n = self.getOption('resolution')
            p0 = self.getOption('point1')
            p1 = self.getOption('point2')
            dist = np.linalg.norm(np.array(p1) - np.array(p0))
            self._distance = list(np.linspace(0, dist, num=n))

        self._probe.PassCellArraysOn()
        self._probe.SetInputConnection(self.getVTKSource().GetOutputPort())
        f = self._exodus_source.getFilters()[-1].getVTKFilter().GetOutputPort()
        self._probe.SetSourceConnection(f)

    def getDistance(self):
        """
        Return the distance array.
        """
        self.checkUpdateState()
        return self._distance

    def getSample(self, variable):
        """
        Return the sampled data for the given variable.
        """
        self.checkUpdateState()
        self._probe.Update()

        # Return data
        if self._probe.GetOutput().GetPointData().HasArray(variable):
            y = self._probe.GetOutput().GetPointData().GetArray(variable)
        else:
            mooseutils.mooseError('Unable to locate the variable, ' + variable + \
                                  ', in the supplied source data.')
            return []
        return [y.GetValue(i) for i in range(y.GetNumberOfTuples() - 1)]
