#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
import vtk
import mooseutils
from .. import utils
from .. import base

class VolumeAxes(base.ChiggerResultBase):
    """
    A class for displaying the 3D axis around a volume
    """
    @staticmethod
    def getOptions():
        opt = base.ChiggerResult.getOptions()
        opt.add('xaxis', VolumeAxes.getAxisActorOptions(), "The x-axis options.")
        opt.add('yaxis', VolumeAxes.getAxisActorOptions(), "The y-axis options.")
        opt.add('zaxis', VolumeAxes.getAxisActorOptions(), "The z-axis options.")
        return opt

    @staticmethod
    def getAxisActorOptions():
        """
        Return the options for a vtkAxis object.
        """
        opt = utils.Options()
        opt.add('color', [1, 1, 1], "The color of the title, text, ticks, and axis line.")
        opt.add('minor_ticks', False, "Enable/disable the minor tick marks.")
        return opt

    def __init__(self, result, **kwargs):
        super(VolumeAxes, self).__init__(renderer=result.getVTKRenderer(), **kwargs)

        self._vtkactor = vtk.vtkCubeAxesActor()
        self._result = result

    def reset(self):
        """
        Remove the vtkCubeAxesActor.
        """
        super(VolumeAxes, self).reset()
        self._vtkrenderer.RemoveViewProp(self._vtkactor)

    def initialize(self):
        """
        Add the actor to renderer.
        """
        super(VolumeAxes, self).initialize()
        self._vtkrenderer.AddViewProp(self._vtkactor)

    def update(self, **kwargs):
        """
        Update vtkCubeAxesActor object to cover extents of result.
        """
        super(VolumeAxes, self).update(**kwargs)

        if self._result.needsUpdate():
            self._result.update()

        xmin, xmax = utils.get_bounds(*self._result.getSources())
        bnds = [xmin[0], xmax[0], xmin[1], xmax[1], xmin[2], xmax[2]]
        self._vtkactor.SetBounds(*bnds)

        self._vtkactor.SetCamera(self._vtkrenderer.GetActiveCamera())

        self.__updateAxisOptions('x')
        self.__updateAxisOptions('y')
        self.__updateAxisOptions('z')

        self._vtkactor.SetGridLineLocation(vtk.vtkCubeAxesActor.VTK_GRID_LINES_FURTHEST)

    def __updateAxisOptions(self, axis):
        """
        Helper for updating Axis level settings.
        """
        if axis not in ['x', 'y', 'z']:
            mooseutils.mooseError("Must provide 'x', 'y', or 'z'.")
            return

        opt = self.getOption(axis + 'axis')
        color = opt['color']
        comp = ['x', 'y', 'z'].index(axis)
        self._vtkactor.GetTitleTextProperty(comp).SetColor(*color)
        self._vtkactor.GetLabelTextProperty(comp).SetColor(*color)

        func = getattr(self._vtkactor, 'Set{}AxisMinorTickVisibility'.format(axis.upper()))
        func(opt['minor_ticks'])

        func = getattr(self._vtkactor, 'Get{}AxesLinesProperty'.format(axis.upper()))
        func().SetColor(*color)
