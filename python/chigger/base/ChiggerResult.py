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
from chigger import utils
from ChiggerResultBase import ChiggerResultBase
from ChiggerSourceBase import ChiggerSourceBase

class ChiggerResult(ChiggerResultBase):
    """
    A ChiggerResult object capable of attaching an arbitrary number of ChiggerFilterSourceBase
    objects to the vtkRenderer.

    Any options supplied to this object are automatically passed down to the ChiggerFilterSourceBase
    objects contained by this class, if the applicable. To have the settings of the contained source
    objects appear in this objects option dump then simply add the settings to the static
    validOptions method of the derived class. This is not done here because this class is designed
    to accept arbitrary ChiggerFilterSourceBase object which may have varying settings, see
    ExodusResult for an example of a single type implementation based on this class.

    Inputs:
        *sources: A tuple of ChiggerFilterSourceBase object to render.
        **kwargs: see ChiggerResultBase
    """
    # The Base class type that this object to which its ownership is restricted.
    SOURCE_TYPE = ChiggerSourceBase

    @staticmethod
    def validOptions():
        opt = ChiggerResultBase.validOptions()
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = ChiggerResultBase.validKeyBindings()
        bindings.add('o', lambda s, *args: ChiggerResultBase.printOptions(s),
                     desc="Display the available key, value options for this result.")
        bindings.add('o', lambda s, *args: ChiggerResultBase.printSetOptions(s), shift=True,
                     desc="Display the available key, value options as a 'setOptions' method call.")
        return bindings

    def __init__(self, *sources, **kwargs):
        super(ChiggerResult, self).__init__(renderer=kwargs.pop('renderer', None), **kwargs)

        self._sources = list()
        for src in sources:
            self._addSource(src)

        self.setOptions(**kwargs)

    def init(self, window):
        """
        Initialize the result object with the RenderWindow.
        """
        super(ChiggerResult, self).init(window)
        for src in self._sources:
            self._vtkrenderer.AddActor(src.getVTKActor())

    def deinit(self):
        """
        Clean up the object prior to removal from RenderWindow.
        """
        for src in self._sources:
            self._vtkrenderer.RemoveActor(src.getVTKActor())

    def getSources(self):
        """
        Return the list of ChiggerSource objects.
        """
        return self._sources

    def _addSource(self, source):
        """
        Add a new chigger source object to the result.
        """
        if not isinstance(source, self.SOURCE_TYPE):
            msg = 'The supplied source type of {} must be of type {}.'
            raise mooseutils.MooseException(msg.format(source.__class__.__name__,
                                                       self.SOURCE_TYPE.__name__))

        self._sources.append(source)
        source.setVTKRenderer(self._vtkrenderer)
        source._ChiggerSourceBase__result = self #pylint: disable=protected-access

    def getBounds(self):
        """
        Return the bounding box of the results.
        """
        return utils.get_vtk_bounds_min_max(*[src.getBounds() for src in self._sources])

    def setOptions(self, *args, **kwargs):
        """
        Apply the supplied options to this object and the contained ChiggerFilterSourceBase objects.
        (override)

        Inputs:
            see ChiggerResultBase
        """
        super(ChiggerResult, self).setOptions(*args, **kwargs)
        for src in self._sources:
            valid = src.validOptions()
            if args:
                for sub in args:
                    if sub in valid:
                        src.setOptions(sub, **kwargs)
            else:
                for key, value in kwargs.iteritems():
                    if key in src._options: #pylint: disable=protected-access
                        src.setOption(key, value)

    def setOption(self, key, value):
        """
        Set an individual option for this class and associated source objects.
        """
        super(ChiggerResult, self).setOption(key, value)
        for src in self._sources:
            if key in src._options: #pylint: disable=protected-access
                src.setOption(key, value)

    def update(self, **kwargs):
        """
        Update this object and the contained ChiggerFilterSourceBase objects. (override)

        Inputs:
            see ChiggerResultBase
        """
        super(ChiggerResult, self).update(**kwargs)
        for src in self._sources:
            src.update(**kwargs)

    def getRange(self, local=False):
        """
        Return the min/max range for the selected variables and blocks/boundary/nodeset.

        NOTE: For the range to be restricted by block/boundary/nodest the reader must have
              "squeeze=True", which can be much slower.
        """
        rngs = [src.getRange(local=local) for src in self._sources]
        return utils.get_min_max(*rngs)

    def __iter__(self):
        """
        Provides iteration access to the underlying source objects.
        """
        for src in self._sources:
            yield src

    def __getitem__(self, index):
        """
        Provide [] access to the source objects.
        """
        self.update()
        return self._sources[index]

    def __len__(self):
        """
        The number of source objects.
        """
        return len(self._sources)
