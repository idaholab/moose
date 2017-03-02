"""ExodusResultLineSampler"""
from ExodusSourceLineSampler import ExodusSourceLineSampler
from ..base import ChiggerResult

class ExodusResultLineSampler(ChiggerResult):
    """
    Object for sampling ExodusSource object contained in an ExodusResult.
    """

    @staticmethod
    def getOptions():
        opt = ChiggerResult.getOptions()
        return opt

    def __init__(self, exodus_result, **kwargs):

        self._exodus_result = exodus_result
        sources = []
        for src in self._exodus_result:
            sources.append(ExodusSourceLineSampler(src, **kwargs))

        super(ExodusResultLineSampler, self).__init__(*sources, renderer=exodus_result.getVTKRenderer(), viewport=exodus_result.getOption('viewport'), **kwargs)
