#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import vtk
from .. import base
class Annotation(base.ChiggerSource2D):
    """
    Base class for 2D annotations.
    """
    @staticmethod
    def validParams():
        opt = base.ChiggerSource2D.validParams()
        opt.add('position', (0, 0), vtype=(int, float), size=2,
                doc="The text position in normalized viewport coordinates.")
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = base.ChiggerSource2D.validKeyBindings()
        bindings.add('right', Annotation._move, args=(0.01, 0),
                     desc="Move the object to the right.")
        bindings.add('left', Annotation._move, args=(-0.01, 0),
                     desc="Move the object to the left.")
        bindings.add('up', Annotation._move, args=(0, 0.01),
                     desc="Move the object up.")
        bindings.add('down', Annotation._move, args=(0, -0.01),
                     desc="Move the object down.")
        return bindings

    def _move(self, dx, dy):
        pos = self.getParam('position')
        self.setParam('position', (pos[0] + dx, pos[1] + dy))
        self.printOption('position')
