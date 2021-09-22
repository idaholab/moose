#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from ExodusSource import ExodusSource
from ExodusReader import ExodusReader
from MultiAppExodusReader import MultiAppExodusReader
from moosetools import mooseutils
from .. import base
from .. import utils

class ExodusResult(base.ChiggerResult):
    """
    Result object to displaying ExodusII data from a single reader.
    """
    @staticmethod
    def validParams():
        opt = base.ChiggerResult.validParams()
        opt += ExodusSource.validParams()
        opt.add('explode', None, "When multiple sources are being used (e.g., NemesisReader) "
                                 "setting this to a value will cause the various sources to be "
                                 "'exploded' away from the center of the entire object.",
                vtype=(int, float))
        opt.add('local_range', False, "Use local range when computing the default data range.")

        return opt

    @staticmethod
    def validKeyBindings():

        bindings = base.ChiggerResult.validKeyBindings()

        # Opacity keybindings
        bindings.add('a', ExodusResult._updateOpacity,
                     desc='Increase the alpha (opacity) by 1%.')
        bindings.add('a', ExodusResult._updateOpacity, shift=True,
                     desc='Decrease the alpha (opacity) by 1%.')

        # Colormap keybindings
        bindings.add('m', ExodusResult._updateColorMap,
                     desc="Toggle through available colormaps.")
        bindings.add('m', ExodusResult._updateColorMap, shift=True,
                     desc="Toggle through available colormaps, in reverse direction.")

        # Time keybindngs
        bindings.add('t', ExodusResult._updateTimestep,
                     desc="Increase timestep by 1.")
        bindings.add('t', ExodusResult._updateTimestep, shift=True,
                     desc="Decrease the timestep by 1.")

        return bindings

    def __init__(self, reader, **kwargs):

        self._reader = reader

        # Build the ExodusSource objects
        if isinstance(reader, ExodusReader):
            sources = [ExodusSource(reader)]
        elif isinstance(reader, MultiAppExodusReader):
            sources = [ExodusSource(r) for r in reader]
        else:
            raise mooseutils.MooseException('The supplied object of type {} is not supported, '
                                            'only ExodusReader and MultiAppExodusReader objects '
                                            'may be utilized.'.format(reader.__class__.__name__))

        # Supply the sources to the base class
        super(ExodusResult, self).__init__(*sources, **kwargs)

        self.__outline_result = None

    def update(self, **kwargs):
        super(ExodusResult, self).update(**kwargs)

        # Update the ExodusReader objects
        self._reader.update()

        # Do not mess with the range if there is a source without a variable
        if any([src.getCurrentVariableInformation() is None for src in self._sources]):
            return

        # Re-compute ranges for all sources
        rng = list(self.getRange(local=self.getParam('local_range')))
        if self.isValid('range'):
            rng = self.getParam('range')
        else:
            if self.isValid('min'):
                rng[0] = self.getParam('min')
            if self.isValid('max'):
                rng[1] = self.getParam('max')

        if rng[0] > rng[1]:
            mooseutils.mooseDebug("Minimum range greater than maximum:", rng[0], ">", rng[1],
                                  ", the range/min/max settings are being ignored.")
            rng = list(self.getRange())

        for src in self._sources:
            src.getVTKMapper().SetScalarRange(rng)

        # Explode
        if self.isValid('explode'):
            factor = self.applyOption('explode')
            m = self.getCenter()
            for src in self._sources:
                c = src.getVTKActor().GetCenter()
                d = (c[0]-m[0], c[1]-m[1], c[2]-m[2])
                src.getVTKActor().AddPosition(d[0]*factor, d[1]*factor, d[2]*factor)

    def getRange(self, **kwargs):
        """
        Return the min/max range for the selected variables and blocks/boundary/nodeset.

        NOTE: For the range to be restricted by block/boundary/nodest the reader must have
              "squeeze=True", which can be much slower.
        """
        rngs = [src.getRange(**kwargs) for src in self._sources]
        return utils.get_min_max(*rngs)

    def getCenter(self):
        """
        Return the center (based on the bounds) of all the objects.
        """
        b = self.getBounds()
        return ((b[1]-b[0])/2., (b[3]-b[2])/2., (b[5]-b[4])/2.)

    def _updateOpacity(self, window, binding): #pylint: disable=unused-argument
        opacity = self.getParam('opacity')
        if binding.shift:
            if opacity > 0.05:
                opacity -= 0.05
        else:
            if opacity <= 0.95:
                opacity += 0.05
        self.update(opacity=opacity)
        self.printOption('opacity')

    def _updateColorMap(self, window, binding): #pylint: disable=unused-argument
        step = 1 if not binding.shift else -1
        available = self._sources[0]._colormap.names() #pylint: disable=protected-access
        index = available.index(self.getParam('cmap'))

        n = len(available)
        index += step
        if index == n:
            index = 0
        elif index < 0:
            index = n - 1

        self.setParam('cmap', available[index])
        self.printOption('cmap')

    def _updateTimestep(self, window, binding): #pylint: disable=unused-argument
        step = 1 if not binding.shift else -1
        current = self._reader.getTimeData().timestep + step
        n = len(self._reader.getTimes())
        if current == n:
            current = 0
        self._reader.setParam('time', None)
        self._reader.setParam('timestep', current)
        self._reader.printOption('timestep')
