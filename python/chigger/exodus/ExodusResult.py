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
from ExodusSource import ExodusSource
from ExodusReader import ExodusReader
from MultiAppExodusReader import MultiAppExodusReader
import mooseutils
from .. import base

class ExodusResult(base.ChiggerResult):
    """
    Result object to displaying ExodusII data from a single reader.
    """
    @staticmethod
    def getOptions():
        opt = base.ChiggerResult.getOptions()
        opt += ExodusSource.getOptions()
        return opt

    def __init__(self, reader, **kwargs):

        # Build the ExodusSource objects
        if isinstance(reader, ExodusReader):
            sources = [ExodusSource(reader)]
        elif isinstance(reader, MultiAppExodusReader):
            sources = [ExodusSource(r) for r in reader]
        else:
            raise mooseutils.MooseException('The supplied object of type {} is not supported, '
                                            'only ExodusReader and MultiAppExodusReader objects '
                                            'may be utilized.'.format(reader.__class__.__name__))

        # Supply the sources to the base class
        super(ExodusResult, self).__init__(*sources, **kwargs)
