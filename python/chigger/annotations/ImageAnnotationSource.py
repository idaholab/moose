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

class ImageAnnotationSource(base.ChiggerSource):
    """
    Source for displaying images in 3D space.
    """
    VTKACTOR_TYPE = vtk.vtkImageActor
    VTKMAPPER_TYPE = vtk.vtkImageSliceMapper

    @staticmethod
    def getOptions():
        """
        Return default options for this object.
        """
        opt = base.ChiggerSource.getOptions()
        opt.add('filename', None, "The PNG file to read, this can be absolute or relative path to "
                                  "a PNG or just the name of a PNG located in the chigger/logos "
                                  "directory.", vtype=str)
        return opt

    def __init__(self, **kwargs):
        super(ImageAnnotationSource, self).__init__(vtkactor_type=vtk.vtkImageActor,
                                                    vtkmapper_type=vtk.vtkImageSliceMapper,
                                                    **kwargs)

        self.__reader = vtk.vtkPNGReader()

    def getVTKSource(self):
        """
        Return the image reader object. (override)
        """
        return self.__reader

    def update(self, **kwargs):
        """
        Updates the image reader. (override)
        """
        super(ImageAnnotationSource, self).update(**kwargs)

        if self.isOptionValid('filename'):
            filename = self.getOption('filename')
            if not os.path.exists(filename):
                filename = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'logos',
                                                        os.path.basename(filename)))
            if not os.path.exists(filename):
                raise mooseutils.MooseException('Unable to locate image file: {}'.format(filename))
            self.__reader.SetFileName(filename)
            self.__reader.Update()
