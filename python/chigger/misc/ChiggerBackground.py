"""ChiggerBackground"""
from .. import base
class ChiggerBackground(base.ChiggerResultBase):
    """
    An empty renderer to serve as the background for other objects.
    """
    @staticmethod
    def getOptions():
        opt = base.ChiggerResultBase.getOptions()
        opt.setDefault('layer', 0)
        return opt

    def __init__(self, **kwargs):
        super(ChiggerBackground, self).__init__(**kwargs)
