#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .ImageAnnotation import ImageAnnotation
from .TextAnnotationSource import TextAnnotationSource
from .TimeAnnotationSource import TimeAnnotationSource
from .. import base
TextAnnotation = base.create_single_source_result(TextAnnotationSource)
TimeAnnotation = base.create_single_source_result(TimeAnnotationSource)
