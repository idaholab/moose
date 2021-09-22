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
from moosetools import mooseutils
from .. import base
from .. import utils

class VolumeAxesSource(base.ChiggerSourceBase):

    @staticmethod
    def validParams():
        opt = base.ChiggerResult.validParams()
        opt.add('xaxis', default=VolumeAxesSource.getAxisActorParams(), doc="The x-axis options.")
        opt.add('yaxis', default=VolumeAxesSource.getAxisActorParams(), doc="The y-axis options.")
        opt.add('zaxis', default=VolumeAxesSource.getAxisActorParams(), doc="The z-axis options.")
        opt.add('point1', default=(0, 0, 0), vtype=(int, float), size=3,
                doc="Lower corner of bounding box.")
        opt.add('point2', default=(1, 1, 1), vtype=(int, float), size=3,
                doc="Upper corner of bounding box.")
        opt.setValue('interactive', False)
        return opt

    @staticmethod
    def getAxisActorParams():
        """
        Return the options for a vtkAxis object.
        """
        opt = utils.Params()
        opt.add('color', default=(1, 1, 1), vtype=(int, float), size=3,
                doc="The color of the title, text, ticks, and axis line.")
        opt.add('minor_ticks', default=False, vtype=bool,
                doc="Enable/disable the minor tick marks.")
        return opt

    def __init__(self, **kwargs):
        super(VolumeAxesSource, self).__init__(vtkactor_type=vtk.vtkCubeAxesActor,
                                               vtkmapper_type=None,
                                               **kwargs)

    def update(self, **kwargs):
        """
        Update vtkCubeAxesActor object to cover extents of result.
        """
        super(VolumeAxesSource, self).update(**kwargs)

        p0 = self.getParam('point1')
        p1 = self.getParam('point2')
        bnds = [p0[0], p1[0], p0[1], p1[1], p0[2], p1[2]]
        self._vtkactor.SetBounds(*bnds)

        self._vtkactor.SetCamera(self._vtkrenderer.GetActiveCamera())
        self.__updateAxisParams('x')
        self.__updateAxisParams('y')
        self.__updateAxisParams('z')

        self._vtkactor.SetGridLineLocation(vtk.vtkCubeAxesActor.VTK_GRID_LINES_FURTHEST)

    def getBounds(self):
        return self._vtkactor.GetBounds()

    def __updateAxisParams(self, axis):
        """
        Helper for updating Axis level settings.
        """
        if axis not in ['x', 'y', 'z']:
            mooseutils.mooseError("Must provide 'x', 'y', or 'z'.")
            return

        opt = self.getParam(axis + 'axis')
        color = opt.getValue('color')
        comp = ['x', 'y', 'z'].index(axis)
        self._vtkactor.GetTitleTextProperty(comp).SetColor(*color)
        self._vtkactor.GetLabelTextProperty(comp).SetColor(*color)

        func = getattr(self._vtkactor, 'Set{}AxisMinorTickVisibility'.format(axis.upper()))
        func(opt.getValue('minor_ticks'))

        func = getattr(self._vtkactor, 'Get{}AxesLinesProperty'.format(axis.upper()))
        func().SetColor(*color)
