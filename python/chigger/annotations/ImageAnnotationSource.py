#pylint: disable=missing-docstring
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

import mooseutils
from .. import base

class ImageAnnotationSource(base.ChiggerSource2D):
    """
    Source for displaying images in 3D space.
    """

    @staticmethod
    def validOptions():
        """
        Return default options for this object.
        """
        opt = base.ChiggerSource.validOptions()
        opt.add('filename', None, "The PNG file to read, this can be absolute or relative path to "
                                  "a PNG or just the name of a PNG located in the chigger/logos "
                                  "directory.", vtype=str)
        opt.add('width', vtype=float,
                doc="The image width as a fraction of the window width "
                    "(None maintains image dimension).")
        opt.add('height', vtype=float,
                doc="The image height as a fraction of the window width "
                    "(None maintains image dimension).")
        opt.add('position', (0.5, 0.5), vtype=float, size=2,
                doc="The position of the image within the viewport, in relative coordinates.")
        opt.add('horizontal_alignment', 'center', allow=('left', 'center', 'right'),
                doc="The position horizontal position alignment.")
        opt.add('vertical_alignment', 'center', allow=('bottom', 'center', 'top'),
                doc="The position vertical position alignment.")
        return opt

    def __init__(self, **kwargs):
        super(ImageAnnotationSource, self).__init__(vtkmapper_type=vtk.vtkImageMapper,
                                                    **kwargs)

        self.__reader = vtk.vtkPNGReader()
        self.__resize = vtk.vtkImageResize()
        self.__resize.SetInputConnection(self.__reader.GetOutputPort())

        # This is required to get the image to appear
        self._vtkmapper.SetColorWindow(255)  # width of the color range to map to
        self._vtkmapper.SetColorLevel(127.5) # center of the color range to map to

    def getVTKSource(self):
        """
        Return the image reader object. (override)
        """
        return self.__resize

    def update(self, **kwargs):
        """
        Updates the image reader. (override)
        """
        super(ImageAnnotationSource, self).update(**kwargs)

        # Load the filename
        if self.isOptionValid('filename'):
            filename = self.applyOption('filename')
            if not os.path.exists(filename):
                filename = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'logos',
                                                        os.path.basename(filename)))
            if not os.path.exists(filename):
                raise mooseutils.MooseException('Unable to locate image file: {}'.format(filename))
            self.__reader.SetFileName(filename)

        # Set the width/height
        if self.isOptionValid('width') or self.isOptionValid('height'):
            window_size = self._vtkrenderer.GetSize()
            self.__reader.Update()
            image_size = list(self.__reader.GetOutput().GetDimensions())
            aspect = float(image_size[0]) / float(image_size[1]) # w/h

            if self.isOptionValid('width') and self.isOptionValid('height'):
                image_size[0] = int(window_size[0] * self.applyOption('width'))
                image_size[1] = int(window_size[1] * self.applyOption('height'))

            elif self.isOptionValid('width'):
                image_size[0] = int(window_size[0] * self.applyOption('width'))
                image_size[1] = int(image_size[0] / aspect)

            elif self.isOptionValid('height'):
                image_size[1] = int(window_size[1] * self.applyOption('height'))
                image_size[0] = int(image_size[1] * aspect)

            self.__resize.SetOutputDimensions(*image_size)

        # Image position
        if self.isOptionValid('position'):

            # Determine the position in pixels
            tr = vtk.vtkCoordinate()
            tr.SetCoordinateSystemToNormalizedViewport()
            p = self.applyOption('position')
            tr.SetValue(p[0], p[1], 0)
            position = list(tr.GetComputedDisplayValue(self._vtkrenderer))

            # Get the image size
            image_size = self.__resize.GetOutputDimensions()
            if image_size == (-1, -1, -1):
                self.__reader.Update()
                image_size = self.__reader.GetOutput().GetDimensions()

            # Adjust the position for alignment
            if self.getOption('horizontal_alignment') == 'center':
                position[0] = position[0] - (image_size[0]*0.5)
            elif self.getOption('horizontal_alignment') == 'right':
                position[0] = position[0] - image_size[0]

            if self.getOption('vertical_alignment') == 'center':
                position[1] = position[1] - (image_size[1]*0.5)
            elif self.getOption('vertical_alignment') == 'top':
                position[1] = position[1] - image_size[1]

            self._vtkactor.SetPosition(*position)

    def getBounds(self):
        return self.getVTKActor().GetBounds()
