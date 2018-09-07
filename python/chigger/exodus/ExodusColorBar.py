#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import mooseutils
from .. import base
from .. import misc

class ExodusColorBar(misc.ColorBar):
    """
    ColorBar designed to work with ExodusResult objects.

    Inputs:
        result0: The ExodusResult objects for the primary axis.
        result1: (Optional) The ExodusResult for the secondary axis.
    """
    @staticmethod
    def validOptions():
        opt = misc.ColorBar.validOptions()
        opt.set('viewport', None)
        opt.set('layer', None)
        return opt

    def __init__(self, *results, **kwargs):
        self._results = results
        if len(results) not in [1, 2]:
            raise mooseutils.MooseException('One or two ExodusResult objects must be supplied to '
                                            'the ExodusColorBar')

        super(ExodusColorBar, self).__init__(**kwargs)

    def getResult(self, index=0):
        """
        Return the associated ExodusResult object. The index must be 0 or 1.
        """
        if index not in [0, 1]:
            raise mooseutils.MooseException("The supplied index must be 0 or 1.")
        return self._results[index]

    def setOptions(self, *args, **kwargs):
        """
        Update the supplied options and apply the colormap options from the ExodusResult.
        """
        if not args:
            opts = base.ColorMap.validOptions()
            for result in self._results:
                for key in opts.keys():
                    kwargs[key] = result.getOption(key)

            if self.getOption('viewport') is None:
                self.setOption('viewport', self._results[0].getOption('viewport'))
            if self.getOption('layer') is None:
                self.setOption('layer', self._results[0].getOption('layer'))

        super(ExodusColorBar, self).setOptions(*args, **kwargs)

    def update(self, **kwargs):
        """
        Extracts the settings from the ExodusResult object to define the correct settings for the
        colorbar.
        """
        n = len(self._results)
        primary = self.getOption('primary')
        secondary = self.getOption('secondary')

        def set_axis_options_helper(ax, result): #pylint: disable=invalid-name
            """Helper for setting axis options."""
            ax.set('lim', result[0].getVTKMapper().GetScalarRange())
            if ax.get('title') is None:
                ax.set('title', result[0].getVTKMapper().GetArrayName())

        # Primary
        if n > 0:
            set_axis_options_helper(primary, self._results[0])

        # Secondary
        if n == 2:
            set_axis_options_helper(secondary, self._results[1])
            secondary.set('visible', True)

        self.setOption('primary', primary)
        self.setOption('secondary', secondary)
        super(ExodusColorBar, self).update(**kwargs)
