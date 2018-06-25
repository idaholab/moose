#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mooseutils
from .. import misc

class ExodusColorBar(misc.ColorBar):
    """
    ColorBar designed to work with ExodusResult objects.

    Inputs:
        result0: The ExodusResult objects for the primary axis.
        result1: (Optional) The ExodusResult for the secondary axis.
    """

    AXIS_NAMES = ['primary', 'secondary']

    @staticmethod
    def getOptions():
        opt = misc.ColorBar.getOptions()
        opt.setDefault('viewport', None)
        return opt

    def __init__(self, *results, **kwargs):
        super(ExodusColorBar, self).__init__(**kwargs)

        self._results = results
        if len(results) not in [1, 2]:
            raise mooseutils.MooseException('One or two ExodusResult objects must be supplied to '
                                            'the ExodusColorBar')

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
        opts = ['cmap', 'cmap_reverse', 'cmap_num_colors', 'cmap_range']
        cmap_options = {key:self._results[0].getOption(key) for key in opts}
        kwargs.update(cmap_options)

        super(ExodusColorBar, self).setOptions(*args, **kwargs)

    def needsUpdate(self):
        """
        Check if the result ranges has changed.
        """
        for i, result in enumerate(self._results):
            axis_options = self.getOption(self.AXIS_NAMES[i])
            rng = result[0].getVTKMapper().GetScalarRange()
            if rng != axis_options['lim']:
                return True

        return super(ExodusColorBar, self).needsUpdate() or \
               any([result.needsUpdate() for result in self._results])

    def update(self, **kwargs):
        """
        Extracts the settings from the ExodusResult object to define the correct settings for the
        colorbar.
        """

        # Set the options provided
        self.setOptions(**kwargs)
        if self.needsInitialize():
            self.initialize()

        # The results must be updated for the settings to be applied below
        for result in self._results:
            if result.needsUpdate():
                result.update()

        # Enable the secondary if two results provided
        if len(self._results) == 2:
            self.getOption(self.AXIS_NAMES[1])['visible'] = True

        # Apply settings from results
        for i, result in enumerate(self._results):

            # Set the range for the axis' and titles
            axis_options = self.getOption(self.AXIS_NAMES[i])
            axis_options['lim'] = list(result[0].getVTKMapper().GetScalarRange())
            if not axis_options.isOptionValid('title'):
                self._sources[i+1].getVTKSource().SetTitle(result[0].getVTKMapper().GetArrayName())

            # Viewport
            if not self.isOptionValid('viewport'):
                self.setOption('viewport', result.getOption('viewport'))
            self.setOption('layer', result.getOption('layer'))

        super(ExodusColorBar, self).update(**kwargs)
