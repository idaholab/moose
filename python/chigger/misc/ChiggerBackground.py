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
    An empty vtkRenderer to serve as the background for other objects.
    """
    @staticmethod
    def validOptions():
        opt = base.ChiggerResultBase.validOptions()
        opt.add('background', (0., 0., 0.), vtype=float, size=3,
                doc="The background color, only applied when the 'layer' option is zero. A " \
                    "background result is automatically added when chigger.RenderWindow is " \
                    "utilized.")
        opt.add('background2', None, vtype=float, size=3,
                doc="The second background color, when supplied this creates a gradient " \
                    "background, only applied when the 'layer' option is zero. A background " \
                    "result is automatically added when chigger.RenderWindow is utilized.")
        opt.add('gradient_background', False, doc="Enable/disable the use of a gradient background.")

        opt.set('layer', 0)
        opt.set('interactive', False)
        return opt

    def __init__(self, **kwargs):
        super(ChiggerBackground, self).__init__(**kwargs)


    def update(self, **kwargs):
        super(ChiggerBackground, self).update(**kwargs)

        if self.getOption('layer') != 0:
            raise ValueError("The 'layer' option must be set to zero for background settings to apply.")

        if self.isOptionValid('background'):
            self._vtkrenderer.SetBackground(self.applyOption('background'))

        if self.isOptionValid('background2'):
            self._vtkrenderer.SetBackground2(self.applyOption('background2'))

        if self.isOptionValid('gradient_background'):
            self._vtkrenderer.SetGradientBackground(self.applyOption('gradient_background'))
