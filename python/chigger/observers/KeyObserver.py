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
from ChiggerObserver import ChiggerObserver
class KeyObserver(ChiggerObserver):
    """
    Class for creating key press observers to be passed in to RenderWindow object.
    """
    @staticmethod
    def getOptions():
        opt = ChiggerObserver.getOptions()
        return opt

    def __init__(self, **kwargs):
        super(KeyObserver, self).__init__(**kwargs)
        self._key = None

    def init(self, window):
        """
        Add the KeyPressEvent for this object.
        """
        super(KeyObserver, self).init(window)
        window.getVTKInteractor().AddObserver('KeyPressEvent', self._callback)

    def _callback(self, obj, event): #pylint: disable=unused-argument
        """
        The function to be called by the RenderWindow.

        Inputs:
            obj, event: Required by VTK.
        """
        self._key = obj.GetKeySym()
        self.update()
        self._key = None
