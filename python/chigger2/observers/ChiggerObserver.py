#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import logging
import weakref
#from vtk.util.vtkAlgorithm import VTKPythonAlgorithmBase
from .. import utils
from .. import base

class ChiggerObserver(base.ChiggerObject):
    """
    Base class for defining VTK observer functions.

    This object is a base class and not intended for general use, see TimerObserver as an example.
    """
    @staticmethod
    def validParams():
        opt = base.ChiggerObject.validParams()
        opt.add('window', default=utils.get_current_window(), required=True,
                doc='The chigger.Window object that this Viewport is to be associated')
        return opt

    def __init__(self, **kwargs):
        base.ChiggerObject.__init__(self, **kwargs)

        # Storing a direct reference (self._window = window) causes VTK to seg. fault. As far as I
        # can tell this is due to a cyclic reference between the observers added to the VTK
        # interactor objects. Using a weak reference with the self._window property allows this
        # class to operate like desired.
        self.__window_weakref = weakref.ref(self.getParam('window'))


        self.__observer_tags = list()

        #base.ChiggerAlgorithm.__init__(self, **kwargs)

        #self.SetNumberOfInputPorts(1)
        #self.SetNumberOfOutputPorts(0)
        #self.InputType = 'vtkPythonAlgorithm'
        #self.SetInputConnection(window.GetOutputPort(0))


    #def RequestData(self, request, inInfo, outInfo):

        #inp = inInfo[0].GetInformationObject(0).Get(vtk.vtkDataObject.DATA_OBJECT())
        #self.SetInputData(inp)
    #    return 1
    #def init(self, window):
    #    """
    #    Initialize the observer, this is called by the RenderWindow automatically.

    #    NOTE: This is an internal method, do not call it.
    #    """
    #    self._window = window

    def addObserver(self, event, function):
        tag = self._window.getVTKInteractor().AddObserver(event, function)
        self.__observer_tags.append(tag)
        return  tag

    def __del__(self):
        pass
        #for tag in self.__observer_tags:
        #    self._window.getVTKInteractor().RemoveObserver(tag)

    @property
    def _window(self):
        return self.__window_weakref()

    def terminate(self):
        """
        Terminate the render window.
        """
        self._window.terminate()
