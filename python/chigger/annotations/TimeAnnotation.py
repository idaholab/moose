from TextAnnotationBase import TextAnnotationBase
from TimeAnnotationSource import TimeAnnotationSource

class TimeAnnotation(TextAnnotationBase):

    @staticmethod
    def validOptions():
        opt = TextAnnotationBase.validOptions()
        opt += TimeAnnotationSource.validOptions()
        return opt

    def __init__(self, **kwargs):
        super(TimeAnnotation, self).__init__(TimeAnnotationSource(), **kwargs)
