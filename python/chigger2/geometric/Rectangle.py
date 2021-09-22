import vtk
import numpy as np
import math
from .. import base, utils, filters
from .GeometricSource import GeometricSource2D

class Rectangle(GeometricSource2D):
    VTKSOURCETYPE = vtk.vtkPlaneSource
    PRECISION = 5

    @staticmethod
    def validParams():
        opt = GeometricSource2D.validParams()
        opt += utils.ColorMapParams.validParams()
        #opt.setDefault('cmap', None)
        opt.add('color', vtype=(utils.Color, utils.AutoColor), doc="The color of the rectangle")

        opt.add('xmin', default=0, vtype=(int, float), required=True,
                verify=(lambda v: v>=0 and v<=1, "Value must be in range [0,1]"))
        opt.add('xmax', default=1, vtype=(int, float), required=True,
                verify=(lambda v: v>=0 and v<=1, "Value must be in range [0,1]"))
        opt.add('ymin', default=0, vtype=(int, float), required=True,
                verify=(lambda v: v>=0 and v<=1, "Value must be in range [0,1]"))
        opt.add('ymax', default=1, vtype=(int, float), required=True,
                verify=(lambda v: v>=0 and v<=1, "Value must be in range [0,1]"))


        # TODO: Do I need the ability to make diamonds and stuff???
        #opt.add('origin', default=(0, 0, 0), vtype=float, size=3,
        #        doc='Define the origin of the plane.')
        #opt.add('point1', default=(1, 0, 0), vtype=float, size=3,
        #        doc='Define the first edge of the plane (origin->point1).')
        #opt.add('point2', default=(0, 1, 0), vtype=float, size=3,
        #        doc='Define the second edge of the plane (origin->point2).')
        opt.add('resolution', default=(1, 1), vtype=int, size=2,
                doc="Define the number of subdivisions in the x- and y-direction of the plane.")

        opt.add('rotate', 0, vtype=int, doc="Angle of rotation in degrees, rotate about the center of the rectangle.")

        opt.add('point_data', None, vtype=vtk.vtkFloatArray,
                doc="The VTK point data to attach to the vtkMapper for this object")

        return opt

    def __init__(self, *args, **kwargs):
        GeometricSource2D.__init__(self, *args, **kwargs)

    def _getBounds(self):
        return (self.getParam('xmin'), self.getParam('xmax'),
                self.getParam('ymin'), self.getParam('ymax'))

    def _onRequestInformation(self, *args):
        """
        Set the options for this cube. (public)
        """
        GeometricSource2D._onRequestInformation(self, *args)

        p0 = (self.getParam('xmin'), self.getParam('ymin'), 0)
        p1 = (p0[0], self.getParam('ymax'), 0)
        p2 = (self.getParam('xmax'), p0[1], 0)

        angle = self.getParam('rotate')
        if angle > 0:
            center = ((p1[0] + p2[0])/2., (p1[1] + p2[1])/2.)
            p0 = utils.rotate_point(p0, center, angle)
            p1 = utils.rotate_point(p1, center, angle)
            p2 = utils.rotate_point(p2, center, angle)

        self._vtksource.SetOrigin(p0)
        self._vtksource.SetPoint1(p1)
        self._vtksource.SetPoint2(p2)

        if self.isParamValid('resolution'):
            self._vtksource.SetResolution(*self.getParam('resolution'))

        pdata = self.getParam('point_data')
        if pdata is not None:
            self._vtksource.Update()
            self._vtksource.GetOutput().GetPointData().SetScalars(pdata)
            self._vtkmapper.SetScalarRange(pdata.GetRange())

        if self.isParamValid('color'):
            self.assignParam('color', self._vtkactor.GetProperty().SetColor)
        else:
            print('CMAP')
            cmap = utils.ColorMapParams.applyParams(self.getParam('cmap'))
            self._vtkmapper.SetLookupTable(cmap)
            self._vtkmapper.SetUseLookupTableScalarRange(True)
