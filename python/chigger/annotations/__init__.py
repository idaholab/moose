"""annotations module"""

from ImageAnnotation import ImageAnnotation
from TextAnnotationSource import TextAnnotationSource
from TimeAnnotationSource import TimeAnnotationSource
from .. import base
TextAnnotation = base.create_single_source_result(TextAnnotationSource)
TimeAnnotation = base.create_single_source_result(TimeAnnotationSource)
