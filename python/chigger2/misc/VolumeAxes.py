#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from VolumeAxesSource import VolumeAxesSource
from .. import base

class VolumeAxes(base.ChiggerResult):
    """
    A class for displaying the 3D axis around a volume
    """
    @staticmethod
    def validParams():
        opt = base.ChiggerResult.validParams()
        opt += VolumeAxesSource.validParams()
        opt.remove('point1')
        opt.remove('point2')
        return opt

    def __init__(self, result, **kwargs):
        super(VolumeAxes, self).__init__(VolumeAxesSource(),
                                         renderer=result.getVTKRenderer(),
                                         **kwargs)
        self._result = result

    def update(self, **kwargs):
        bnds = self._result.getBounds()
        self._sources[0].setParam('point1', (bnds[0], bnds[2], bnds[4]))
        self._sources[0].setParam('point2', (bnds[1], bnds[3], bnds[5]))
        super(VolumeAxes, self).update(**kwargs)
