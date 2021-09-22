#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
from .Image import Image

class Logo(Image):
    """
    A helper version of Image for displaying logos from chigger/logos directory
    """
    @staticmethod
    def validParams():
        """
        Return the default options for this object.
        """
        opt = Image.validParams()
        opt.add('logo', vtype=str, required=True, doc="The name of the logo (e.g., 'moose').")
        return opt

    def __init__(self, *args, **kwargs):
        Image.__init__(self, *args, **kwargs)
        self._location = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'logos'))

    def _onRequestInformation(self, *args):
        filename = None
        basename = os.path.join(self._location, '{}'.format(self.getParam('logo').lower()))
        white_logo = '{}_white.png'.format(basename)
        black_logo = '{}_black.png'.format(basename)

        if os.path.isfile(white_logo) and os.path.isfile(black_logo):
            parent = self._viewport if self._viewport.getParam('layer') == 0 else self._viewport._window
            if parent.isParamValid('background', 'color') and (not parent.isParamValid('background', 'color2')):
                bg = parent.getParam('background', 'color')
                filename = white_logo if sum(bg.rgb()) < 1.5 else black_logo

        if filename is None:
            filename = '{}.png'.format(basename)

        self.setParam('filename', filename)
        Image._onRequestInformation(self, *args)
