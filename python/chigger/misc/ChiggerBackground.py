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
from .. import base
class ChiggerBackground(base.ChiggerResultBase):
    """
    An empty renderer to serve as the background for other objects.
    """
    @staticmethod
    def getOptions():
        opt = base.ChiggerResultBase.getOptions()
        opt.setDefault('layer', 0)
        return opt

    def __init__(self, **kwargs):
        super(ChiggerBackground, self).__init__(**kwargs)
