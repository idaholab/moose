#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
