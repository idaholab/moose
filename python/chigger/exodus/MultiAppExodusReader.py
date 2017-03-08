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
import glob
from ExodusReader import ExodusReader
from .. import base

class MultiAppExodusReader(base.ChiggerObject):
    """
    A reader for MultiApp Exodus files.

    This class is simply a wrapper that creates and ExodusReader object for each file found using
    glob from the supplied pattern.

    Inputs:
        pattern[str]: A string containing a glob pattern for MultiApp ExodusII output files from
                      MOOSE.
    """

    @staticmethod
    def getOptions():
        opt = base.ChiggerObject.getOptions()
        return opt

    def __init__(self, pattern, **kwargs):
        super(MultiAppExodusReader, self).__init__(**kwargs)

        self.__readers = []
        for filename in glob.glob(pattern):
            self.__readers.append(ExodusReader(filename, **kwargs))

    def __iter__(self):
        """
        Provide iterator access to the readers.
        """
        for reader in self.__readers:
            yield reader

    def __getitem__(self, index):
        """
        Provide operator[] access to the readers.
        """
        return self.__readers[index]
