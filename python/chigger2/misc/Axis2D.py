#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import copy
import logging
import vtk
from .. import base
from .. import utils

class Axis2D(base.ChiggerSource):
    """
    Creates an Axis with limits, ticks, etc.
    """

    VTKACTORTYPE = vtk.vtkAxisActor2D#vtk.vtkContextActor
    #VTKMAPPERTYPE = vtk.vtkPolyDataMapper2D#None

    __TEXTKEYS__ = utils.TextParams.validParams().keys()

    @staticmethod
    def validParams():
        opt = base.ChiggerSource.validParams()

        # Axis Line Params
        opt.setValue('linewidth', 1)

        # Title and label Params
        # This object includes general properties ('fontcolor', 'fontopacity', ...) as well as
        # title and label specific properties ('title_fontcolor', ..., 'label_fontcolor'). The
        # default 'fontcolor' and 'fontopacity' are removed to allow the the 'color' and 'opacity'
        # to be the default for this options
        opt.add('title', vtype=str, doc="The title for the axis.")
        opt.add('title_position', 0.5, vtype=(int, float),
                doc="Set the title position between 0 (start) and 1 (end).")
        opt.append(utils.TextParams.validParams(), exclude=['opacity', 'color'])
        opt.append(utils.TextParams.validParams(), prefix='title_', unset=True)
        opt.append(utils.TextParams.validParams(), prefix='label_', unset=True)

        #opt += utils.TextParams.validParams(prefix='title', unset=True)
        #opt += utils.TextParams.validParams(prefix='label', unset=True)
        #opt.setValue('fontcolor', None)
        #opt.setValue('fontopacity', None)
        #opt.setValue('fontitalic', False)

        # Position
        opt.add('point1', vtype=(int, float), size=2, doc="The starting position, in relative viewport coordinates, of the axis line.")
        opt.add('point2', vtype=(int, float), size=2, doc="The ending position, in relative viewport coordinates, of the axis line.")

        # Range
        opt.add('range', (0, 1), vtype=(int, float), size=2, doc="The axis numeric range.")
        opt.add('adjust_range', False, vtype=bool, doc="Adjust the range to 'nice' values.")

        # Major ticks
        opt.add('major_nticks', vtype=int, doc="The number of tick marks.")
        opt.add('major_offset', vtype=int, doc="Offset between tick mark and label.")
        opt.add('major_length', 16, vtype=int, doc="Length of the tick marks.")

        # Minor ticks
        opt.add('minor_nticks', vtype=int, doc="The number of tick marks.")
        opt.add('minor_length', 8, vtype=int, doc="Length of the tick marks.")

        # Axis
        opt.add('axis', True, vtype=bool, doc="Show/hide the axis line.")
        opt.add('format', vtype=str, doc="Label format in printf form.")

        return opt

    def __init__(self, **kwargs):
        base.ChiggerSource.__init__(self, nOutputPorts=1, outputType='vtkPolyData', **kwargs)
        #self._vtkactor.UseFontSizeFromPropertyOn()



    def applyParams(self):
        """
        Update the vtkAxis with given settings. (override)

        Inputs:
            see ChiggerFilterSourceBase
        """
        base.ChiggerSource.applyParams(self)

        # Location
        self._vtkactor.SetPoint1(*self.getParam('point1'))
        self._vtkactor.SetPoint2(*self.getParam('point2'))

        # Title
        if self.isParamValid('title'):
            self._vtkactor.SetTitleVisibility(True)
            self._vtkactor.SetTitle(self.getParam('title'))
        else:
            self._vtkactor.SetTitleVisibility(False)
        self.assignParam('title_position', self._vtkactor.SetTitlePosition)

        # The default font color and opacity should match the 'color' and 'opactity' options
        #if not self.isValid('fontcolor'):
        #    self._parameters.setValue('fontcolor', self.getParam('color'))

        #if not self.isValid('fontopacity'):
        #    self._parameters.setValue('fontopacity', self.getParam('opacity'))

        # Set the values of the title_
        for name in self.__TEXTKEYS__:
            for subname in ['title', 'label']:
                tname = '{}_{}'.format(subname, name)
                if not self.isParamValid(tname):
                    self._parameters.setValue(tname, self._parameters.getValue(name))

        utils.TextParams.applyParams(self._vtkactor.GetTitleTextProperty(), self._parameters, 'title')
        utils.TextParams.applyParams(self._vtkactor.GetLabelTextProperty(), self._parameters, 'label')

        # Range
        self.assignParam('range', self._vtkactor.SetRange)
        self.assignParam('adjust_range', self._vtkactor.SetAdjustLabels)

        # Major tick marks
        num = self.getParam('major_nticks')
        if (num is not None) and self.getParam('adjust_range'):
            self.warning("The 'major_number' option is not applied if 'adjust_range' is enabled.")
        if num is not None:
            if (num > 0):
                self._vtkactor.SetNumberOfLabels(num)
                self._vtkactor.SetTickVisibility(True)
            else:
                self._vtkactor.SetTickVisibility(False)

        self.assignParam('major_length', self._vtkactor.SetTickLength)
        self.assignParam('major_offset', self._vtkactor.SetTickOffset)

        # Minor tick marks
        self.assignParam('minor_nticks', self._vtkactor.SetNumberOfMinorTicks)
        self.assignParam('minor_length', self._vtkactor.SetMinorTickLength)

        # Axis Line
        self.assignParam('axis', self._vtkactor.SetAxisVisibility)
        self.assignParam('format', self._vtkactor.SetLabelFormat)
