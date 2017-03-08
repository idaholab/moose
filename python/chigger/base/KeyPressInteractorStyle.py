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
from .. import utils

class KeyPressInteractorStyle(vtk.vtkInteractorStyleMultiTouchCamera):
    """
    An interactor style for capturing key press events in VTK window.
    """
    def __init__(self, parent=None, **kwargs):
        self.AddObserver("KeyPressEvent", self.keyPress)
        super(KeyPressInteractorStyle, self).__init__(parent, **kwargs)

    def keyPress(self, obj, event): #pylint: disable=unused-argument
        """
        Executes when a key is pressed.

        Inputs:
            obj, event: Required by VTK.
        """
        key = obj.GetInteractor().GetKeySym()
        if key == 'c':
            print '\n'.join(utils.print_camera(self.GetDefaultRenderer().GetActiveCamera()))
