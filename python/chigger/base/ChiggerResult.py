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
from .ChiggerResultBase import ChiggerResultBase
from .ChiggerSourceBase import ChiggerSourceBase

class ChiggerResult(ChiggerResultBase):
    """
    A ChiggerResult object capable of attaching an arbitrary number of ChiggerFilterSourceBase
    objects to the vtkRenderer.

    Any options supplied to this object are automatically passed down to the ChiggerFilterSourceBase
    objects contained by this class, if the applicable. To have the settings of the contained source
    objects appear in this objects option dump then simply add the settings to the static getOptions
    method of the derived class. This is not done here because this class is designed to accept
    arbitrary ChiggerFilterSourceBase object which may have varying settings, see ExodusResult for
    an example of a single type implementation based on this class.

    Inputs:
        *sources: A tuple of ChiggerFilterSourceBase object to render.
        **kwargs: see ChiggerResultBase
    """
    # The Base class type that this object to which its ownership is restricted.
    SOURCE_TYPE = ChiggerSourceBase

    @staticmethod
    def getOptions():
        opt = ChiggerResultBase.getOptions()
        return opt

    def __init__(self, *sources, **kwargs):
        super(ChiggerResult, self).__init__(**kwargs)
        self._sources = sources
        for src in self._sources:
            src._parent = self #pylint: disable=protected-access

    def needsUpdate(self):
        """
        Checks if this object or any of the contained ChiggerFilterSourceBase object require update.
        (override)
        """
        return super(ChiggerResult, self).needsUpdate() or \
               any([src.needsUpdate() for src in self._sources])

    def updateOptions(self, *args):
        """
        Apply the supplied option objects to this object and the contained ChiggerFilterSourceBase
        objects. (override)

        Inputs:
            see ChiggerResultBase
        """
        changed = [self.needsUpdate()]
        changed.append(super(ChiggerResult, self).updateOptions(*args))
        for src in self._sources:
            changed.append(src.updateOptions(*args))
        changed = any(changed)
        self.setNeedsUpdate(changed)
        return changed

    def setOptions(self, *args, **kwargs):
        """
        Apply the supplied options to this object and the contained ChiggerFilterSourceBase objects.
        (override)

        Inputs:
            see ChiggerResultBase
        """
        changed = [self.needsUpdate()]
        changed.append(super(ChiggerResult, self).setOptions(*args, **kwargs))
        for src in self._sources:
            changed.append(src.setOptions(*args, **kwargs))
        changed = any(changed)
        self.setNeedsUpdate(changed)
        return changed

    def update(self, **kwargs):
        """
        Update this object and the contained ChiggerFilterSourceBase objects. (override)

        Inputs:
            see ChiggerResultBase
        """
        super(ChiggerResult, self).update(**kwargs)

        for src in self._sources:
            if src.needsUpdate():
                src.update()

    def getSources(self):
        """
        Return the list of ChiggerSource objects.
        """
        return self._sources

    def getBounds(self, check=True):
        """
        Return the bounding box of the results.

        Inputs:
            check[bool]: (Default: True) When True, perform an update check and raise an exception
                                         if object is not up-to-date. This should not be used.

        TODO: For Peacock, on linux check=False must be set, but I am not sure why.
        """
        if check:
            self.checkUpdateState()
        elif self.needsUpdate():
            self.update()
        return utils.get_bounds(*self._sources)

    def getRange(self, local=False):
        """
        Return the min/max range for the selected variables and blocks/boundary/nodeset.

        NOTE: For the range to be restricted by block/boundary/nodest the reader must have
              "squeeze=True", which can be much slower.
        """
        rngs = [src.getRange(local=local) for src in self._sources]
        return utils.get_min_max(*rngs)

    def reset(self):
        """
        Remove actors from renderer.
        """

        super(ChiggerResult, self).reset()
        for src in self._sources:
            self._vtkrenderer.RemoveViewProp(src.getVTKActor())

    def initialize(self):
        """
        Initialize by adding actors to renderer.
        """
        super(ChiggerResult, self).initialize()
        for src in self._sources:
            if not isinstance(src, self.SOURCE_TYPE):
                n = src.__class__.__name__
                t = self.SOURCE_TYPE.__name__
                msg = 'The supplied source type of {} must be of type {}.'.format(n, t)
                raise mooseutils.MooseException(msg)
            src.setVTKRenderer(self._vtkrenderer)
            self._vtkrenderer.AddViewProp(src.getVTKActor())

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
        return self._sources[index]

    def __len__(self):
        """
        The number of source objects.
        """
        return len(self._sources)
