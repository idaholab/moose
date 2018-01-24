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
from ChiggerObject import ChiggerObject

class ChiggerSourceBase(ChiggerObject):
    """
    A base class for creating "source" objects (in VTK something that uses an vtkActor).

    The term "source" is used generically here to represent an entity that is represented with a
    vtkActor. Note, that this class is even more generic in that the required bases (see
    VTKACTOR_TYPE and VTKMAPPER_TYPE) will allow for both 2D and 3D actor/mappers.

    Any Set/Get VTK methods that can be applied to the base types (see VTKACTOR_TYPE and
    VTKMAPPER_TYPE) should be applied in this class, therefore the settings will then propagate to
    all classes deriving from this, hopefully eliminating duplicate code and options.

    Inputs: vtkactor_type: The VTK actor type to build, must be an instance of VTKACTOR_TYPE
    vtkmapper_type: The VTK mapper type to build, must be an instance of VTKMAPPER_TYPE **kwargs:
    The key, value options for this object.
    """
    # The base class actor/mapper that this object to which ownership is restricted
    VTKACTOR_TYPE = vtk.vtkProp
    VTKMAPPER_TYPE = vtk.vtkAbstractMapper

    @staticmethod
    def getOptions():
        opt = ChiggerObject.getOptions()
        opt.add('visible', True, "Toggle the visibility of the object.")
        return opt

    def __init__(self, vtkactor_type=None, vtkmapper_type=None, **kwargs):
        super(ChiggerSourceBase, self).__init__(**kwargs)

        self._vtkactor = vtkactor_type()
        if not isinstance(self._vtkactor, self.VTKACTOR_TYPE):
            n = type(self._vtkactor).__name__
            t = self.VTKACTOR_TYPE.__name__
            raise mooseutils.MooseException('The supplied actor is a {} but must be a {} '
                                            'type.'.format(n, t))

        if vtkmapper_type:
            self._vtkmapper = vtkmapper_type()
            if not isinstance(self._vtkmapper, self.VTKMAPPER_TYPE):
                n = type(self._vtkmapper).__name__
                t = self.VTKMAPPER_TYPE.__name__
                raise mooseutils.MooseException('The supplied mapper is a {} but must be a {} '
                                                'type.'.format(n, t))
        else:
            self._vtkmapper = None

        if self._vtkmapper:
            self._vtkactor.SetMapper(self._vtkmapper)

        self._vtkrenderer = None # This is set automatically by ChiggerResult object.

    def getVTKActor(self):
        """
        Return the constructed vtk actor object. (public)

        Returns:
            An object derived from vtk.vtkProp.
        """
        return self._vtkactor

    def setVTKRenderer(self, vtkrenderer):
        """
        Set the vtkRenderer object. (public)

        Generally, this should not be used. This method is mainly for the ChiggerResult object to
        set.
        """
        self._vtkrenderer = vtkrenderer

    def getVTKMapper(self):
        """
        Return the constructed vtk mapper object. (public)

        Returns:
            An object derived from vtk.vtkAbstractMapper
        """
        return self._vtkmapper

    def update(self, **kwargs):
        """
        Updates the object by connecting the VTK objects. (override)

        Inputs:
            see ChiggerObject
        """
        super(ChiggerSourceBase, self).update(**kwargs)

        if self.isOptionValid('visible'):
            self._vtkactor.SetVisibility(self.getOption('visible'))
