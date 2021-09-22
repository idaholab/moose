#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import vtk
from .Annotation import Annotation
from .. import geometric

class Image(Annotation):
    """
    Result object for displaying images in 3D space.
    """
    VTKMAPPERTYPE = vtk.vtkImageMapper

    @staticmethod
    def validParams():
        """
        Return the default options for this object.
        """
        opt = Annotation.validParams()
        opt.add('filename', vtype=str, doc="The image filename to open.")
        opt.add('width', vtype=(int, float),
                doc="The image width as a fraction of the window width "
                "(None maintains image dimension).")
        opt.add('height', vtype=(int, float),
                doc="The image height as a fraction of the window width "
                    "(None maintains image dimension).")
        opt.add('halign', 'left', allow=('left', 'center', 'right'),
                doc="The position horizontal position alignment.")
        opt.add('valign', 'bottom', allow=('bottom', 'center', 'top'),
                doc="The position vertical position alignment.")
        opt.add('opacity', default=1., vtype=(int, float),
                verify=(lambda v: v>0 and v <=1, "The 'opacity' must be in range (0,1]"),
                doc="The Image opacity in the range of (0, 1]")
        return opt


    @staticmethod
    def validKeyBindings():
        bindings = Annotation.validKeyBindings()
        bindings.add('right', Image._setSize, args=(0.01, 0), shift=True,
                     desc="Make the image wider by 1% of the viewport")
        bindings.add('left', Image._setSize, args=(-0.01, 0), shift=True,
                     desc="Make the image narrower by 1% of the viewport")
        bindings.add('up', Image._setSize, args=(0, 0.01), shift=True,
                     desc="Make the image taller by 1% of the viewport")
        bindings.add('down', Image._setSize, args=(0, -0.01), shift=True,
                     desc="Make the image shorter by 1% of the viewport")
        return bindings


    def __init__(self, *args, **kwargs):
        Annotation.__init__(self, *args,
                            nOutputPorts=1,
                            outputType='vtkImageData',
                            **kwargs)

        self._vtkactor.GetPositionCoordinate().SetCoordinateSystemToNormalizedViewport()
        self._vtkactor.GetPosition2Coordinate().SetCoordinateSystemToNormalizedViewport()

        self._vtkreader = vtk.vtkPNGReader()
        self._vtkresize = vtk.vtkImageResize()
        self._vtkresize.SetInputConnection(self._vtkreader.GetOutputPort())
        self._vtkresize.SetResizeMethodToOutputDimensions()


    def _onRequestInformation(self, *args):

        self._vtkmapper.SetColorWindow(255);        # width of the color range to map to
        self._vtkmapper.SetColorLevel(127.5);       # center of the color range to map to
        self._vtkmapper.SetRenderToRectangle(False) # enables vtkActor2D::SetPosition

        filename = self.getParam('filename')
        if not os.path.exists(filename):
            raise OSError('Unable to locate image file: {}'.format(self.getParam('filename')))
        self._vtkreader.SetFileName(filename)

        self.assignParam('opacity', self._vtkactor.GetProperty().SetOpacity)

        # Set the width/height
        image_size = self._getImageSize()
        position = self._getPosition(image_size)
        self._vtkactor.SetPosition(position[0], position[1])
        self._vtkactor.SetWidth(image_size[0] if image_size[0] <= 1 else 1)
        self._vtkactor.SetHeight(image_size[1] if image_size[1] <= 1 else 1)

        # Resize the image for the current viewport
        tr = vtk.vtkCoordinate()
        tr.SetCoordinateSystemToNormalizedViewport()
        tr.SetValue(image_size[0], image_size[1], 0)
        sz = list(tr.GetComputedDisplayValue(self._viewport.getVTKRenderer()))
        self._vtkresize.SetOutputDimensions(sz[0], sz[1], 0)

        # Base class call, do this at the end to allow for _highlight to get the correct image size
        Annotation._onRequestInformation(self, *args)

        # TODO: I can not figure out why ChiggerSourceBase::__connectFilters is not making this connection
        self._vtkmapper.SetInputConnection(self._vtkresize.GetOutputPort())

    def _onRequestData(self, inInfo, outInfo):
        Annotation._onRequestData(self, inInfo, outInfo)

        # TODO: This should setup the output of this object, but it doesn't do anything. For
        #       some reason the connection to the mapper is failing
        opt = outInfo.GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        opt.ShallowCopy(self._vtkresize.GetOutput())

    def _getPosition(self, image_size):
        # Determine the position in viewport coordinates, accounting for alignment
        position = list(self.getParam('position'))

        # Adjust the position for alignment
        if self.getParam('halign') == 'center':
            position[0] = position[0] - (image_size[0]*0.5)
        elif self.getParam('halign') == 'right':
            position[0] = position[0] - image_size[0]

        if self.getParam('valign') == 'center':
            position[1] = position[1] - (image_size[1]*0.5)
        elif self.getParam('valign') == 'top':
            position[1] = position[1] - image_size[1]

        return position

    def _getImageSize(self):
        """Return the image size in viewport dimensions"""
        window_size = self._viewport.getVTKRenderer().GetSize()
        self._vtkreader.Update()
        extent = self._vtkreader.GetDataExtent()
        image_size = [extent[1]/window_size[0], extent[3]/window_size[1], 0]
        aspect = image_size[0] / image_size[1] # a = w/h

        if self.isParamValid('width') and self.isParamValid('height'):
            image_size[0] = self.getParam('width')
            image_size[1] = self.getParam('height')

        elif self.isParamValid('width'):
            image_size[0] = self.getParam('width')
            image_size[1] = image_size[0] / aspect # h = w / a

        elif self.isParamValid('height'):
            image_size[1] = self.getParam('height')
            image_size[0] = image_size[1] * aspect # w = h * a

        return image_size

    """
    def _highlight(self):
        if self.getParam('highlight'):
            xmin, ymin = self._vtkactor.GetPosition()
            xmax = xmin + self._vtkactor.GetWidth()
            ymax = ymin + self._vtkactor.GetHeight()
            bounds = (xmin, xmax, ymin, ymax)

            if (self._outline is None):
                self._outline = geometric.Outline2D(self._viewport,
                                                    linewidth=3,
                                                    color=(1,1,0),
                                                    pickable=False,
                                                    interactive=False,
                                                    bounds=bounds)
            else:
                self._outline.setParam('bounds', bounds)

        elif (not self.getParam('highlight')) and (self._outline is not None):
            self._outline.remove()
            del self._outline
            self._outline = None
    """

    def _setSize(self, dx, dy):
         """
         Callback for setting the image width.
         """
         window_size = self._viewport.getVTKRenderer().GetSize()
         image_size = self._vtkresize.GetOutputDimensions()
         w = image_size[0] / window_size[0]
         h = image_size[1] / window_size[1]

         if dx != 0:
             self.setParam('width', w + dx)
             self.printOption('width')
         else:
             self.setParam('height', h + dy)
             self.printOption('height')
