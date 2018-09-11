#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import vtk
from .. import base
from . import OutlineSource, OutlineSource2D

class OutlineResult(base.ChiggerResult):

    @staticmethod
    def validOptions():
        opt = base.ChiggerResult.validOptions()
        opt += OutlineSource.validOptions()
        return opt

    def __init__(self, result, **kwargs):
        if isinstance(result, base.ChiggerResult):
            sources = self.__highlightChiggerResult(result)
        elif isinstance(result, base.ChiggerResultBase):
            sources = self.__highlightChiggerResultBase(result)
        super(OutlineResult, self).__init__(*sources, renderer=result.getVTKRenderer(),
                                            viewport=result.getOption('viewport'),
                                            **kwargs)

    def __highlightChiggerResult(self, result): #pylint: disable=no-self-use
        """
        Highlight the individual sources of a ChiggerResult.
        """
        sources = []
        for src in result.getSources():
            if isinstance(src.getVTKActor(), vtk.vtkActor2D):
                sources.append(OutlineSource2D(src))
            else:
                sources.append(OutlineSource(src))
        return sources

    def __highlightChiggerResultBase(self, result): #pylint: disable=no-self-use
        """
        Highlight the viewport of a ChiggerResultBase.
        """
        return [OutlineSource2D(result)]
