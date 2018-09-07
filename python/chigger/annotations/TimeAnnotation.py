#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

from TextAnnotationBase import TextAnnotationBase
from TimeAnnotationSource import TimeAnnotationSource

class TimeAnnotation(TextAnnotationBase):
    """
    Result object for creating time annotations.
    """

    @staticmethod
    def validOptions():
        opt = TextAnnotationBase.validOptions()
        opt += TimeAnnotationSource.validOptions()
        return opt

    def __init__(self, **kwargs):
        super(TimeAnnotation, self).__init__(TimeAnnotationSource(), **kwargs)
