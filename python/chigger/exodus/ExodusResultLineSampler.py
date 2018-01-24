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
from ExodusSourceLineSampler import ExodusSourceLineSampler
from ..base import ChiggerResult

class ExodusResultLineSampler(ChiggerResult):
    """
    Object for sampling ExodusSource object contained in an ExodusResult.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerResult.getOptions()
        return opt

    def __init__(self, exodus_result, **kwargs):

        self._exodus_result = exodus_result
        sources = []
        for src in self._exodus_result:
            sources.append(ExodusSourceLineSampler(src, **kwargs))

        super(ExodusResultLineSampler, self).__init__(*sources,
                                                      renderer=exodus_result.getVTKRenderer(),
                                                      viewport=exodus_result.getOption('viewport'),
                                                      **kwargs)
