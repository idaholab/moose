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
from ChiggerResult import ChiggerResult

class ResultGroup(ChiggerResult):
    """
    An object for containing multiple ChiggerResult objects that share a renderer.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerResult.getOptions()
        return opt

    def __init__(self, **kwargs):
        super(ResultGroup, self).__init__(**kwargs)
        self._results = []

    def getResults(self):
        """
        Return a list of ChiggerResult objects.
        """
        return self._results

    def __iter__(self):
        """
        Provide iterator access to the readers.
        """
        for result in self._results:
            yield result

    def __getitem__(self, index):
        """
        Provide operator[] access to the readers.
        """
        return self._results[index]

    def update(self, **kwargs):
        """
        Call update on all children.
        """
        for result in self._results:
            result.update(**kwargs)

    def setOptions(self, *args, **kwargs):
        """
        Apply options to all results.
        """
        for result in self._results:
            result.setOptions(*args, **kwargs)

    def needsUpdate(self):
        """
        Check if the group needs to be updated.
        """
        return any([result.needsUpdate() for result in self._results])

    def add(self, result, *args, **kwargs):
        """
        Adds a new ChiggerResult object.

        Args:
            result: A ChiggerResult class (not instance) to create.
            args: The arguments to pass into the class.
            kwargs: Key, value pairs to pass into the class.
        """
        kwargs.setdefault('renderer', self.getVTKRenderer())
        self._results.append(result(*args, **kwargs))
