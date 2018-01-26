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
from chigger.base import ChiggerObject
class ChiggerObserver(ChiggerObject):
    """
    Base class for definning VTK observer functions.
    """
    @staticmethod
    def getOptions():
        opt = ChiggerObject.getOptions()
        return opt

    def __init__(self, *args, **kwargs):
        super(ChiggerObserver, self).__init__(*args, **kwargs)
        self._window = None

    def init(self, window):
        self._window = window
