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
import datetime

from TextAnnotationSource import TextAnnotationSource

class TimeAnnotationSource(TextAnnotationSource):
    """
    Source for creating time stamps.
    """

    @staticmethod
    def getOptions():
        """
        Return default options for this object.
        """
        opt = TextAnnotationSource.getOptions()
        opt.add('time', 330667320, "The time to display, in seconds.", vtype=float)
        opt.add('prefix', 'Time:', "The text to display prior to the time string.")
        opt.add('suffix', None, "The text to display after the time string.", vtype=str)
        opt.add('timedelta', False, "Format the time using the python datetime.timedelta")
        opt.setDefault('position', [0.01, 0.01])
        opt.pop('text')
        return opt

    def update(self, **kwargs):
        """
        Converts timestamp to a text string for display. (override)
        """
        super(TimeAnnotationSource, self).update(**kwargs)

        # The time to display
        time = self.getOption('time')

        # Build the text string
        text = []
        if self.isOptionValid('prefix'):
            text.append(self.getOption('prefix'))

        if self.isOptionValid('timedelta') and self.getOption('timedelta'):
            t = datetime.timedelta(seconds=time)
            text.append(str(t))
        else:
            text.append(str(time))

        if self.isOptionValid('suffix'):
            text.append(self.getOption('suffix'))

        self._vtkmapper.GetTextProperty().Modified()
        self._vtkmapper.SetInput(' '.join(text))
