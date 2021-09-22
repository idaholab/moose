#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import re
import copy
import vtk

from moosetools import mooseutils
from .ChiggerObserver import ChiggerObserver
from .. import utils
from .. import geometric

class MainWindowObserver(ChiggerObserver, utils.KeyBindingMixin):
    """
    The main means for interaction with the chigger interactive window.
    """
    RE = re.compile(r"(?P<key>[^\s=]+)=(?P<value>.*?)(?=(?:,\s[^\s=]+=|\Z)|\)\Z)")

    @staticmethod
    def validParams():
        opt = ChiggerObserver.validParams()
        opt += utils.KeyBindingMixin.validParams()
        return opt

    @staticmethod
    def validKeyBindings():
        bindings = utils.KeyBindingMixin.validKeyBindings()

        bindings.add('v', MainWindowObserver.onNextViewport, args=(1,), desc="Select the next viewport.")
        bindings.add('v', MainWindowObserver.onNextViewport, shift=True, args=(-1,),
                     desc="Select the previous viewport.")

        bindings.add('s', MainWindowObserver.onNextSource, args=(1,),
                     desc="Select the next source in the current viewport.")
        bindings.add('s', MainWindowObserver.onNextSource, shift=True, args=(-1,),
                     desc="Select the previous source in the current viewport.")

        bindings.add('p', MainWindowObserver.onPrintParams,
                     desc="Display the available key, value options for the active source or viewport.")
        bindings.add('p', MainWindowObserver.onPrintSetParams, shift=True,
                     desc="Display the non-default key, value options as a 'setParams' method call for the active source of viewport.")

        bindings.add('c', MainWindowObserver.onPrintCamera,
                     desc="Display the camera settings for the current viewport/object.")
        bindings.add('t', MainWindowObserver.onDeactivate, desc="Clear selection(s).")

        bindings.add('q', MainWindowObserver.onQuit, desc="Close the window.")

        return bindings

    def __init__(self, **kwargs):
        ChiggerObserver.__init__(self, **kwargs)
        utils.KeyBindingMixin.__init__(self)

        self.addObserver(vtk.vtkCommand.KeyPressEvent, self._onKeyPressEvent)

        # Disable interaction by default, but honor user specified interaction
        for viewport in self.getViewports():
            v_i = viewport.getParam('interactive') if viewport.parameters().isSetByUser('interactive') else False
            v_h = viewport.getParam('highlight') if viewport.parameters().isSetByUser('highlight') else v_i
            viewport.setParams(highlight=v_h, interactive=v_i)

            for source in viewport.sources():
                s_i = source.getParam('interactive') if source.parameters().isSetByUser('interactive') else False
                s_h = source.getParam('highlight') if source.parameters().isSetByUser('highlight') else s_i
                source.setParams(highlight=s_h, interactive=s_i)

    def getViewports(self):
        """Complete list of available Viewport objects"""
        return [viewport for viewport in self._window.viewports() if viewport.getParam('layer') > 0]

    def getActiveViewport(self):
        """Current active (highlighted) Viewport object"""
        for viewport in self.getViewports():
            if viewport.getParam('highlight'):
                return viewport
        return None

    def setActiveViewport(self, viewport):
        """Activate the supplied viewport and disable all others"""
        for vp in self.getViewports():
            active = viewport is vp
            vp.setParams(interactive=False, highlight=active)
            vp.updateInformation()

    def onNextViewport(self, increment):
        """Activate the "next" viewport object."""
        self.debug('Select Next Viewport')

        # Remove highlighting from the active source.
        self.setActiveSource(None)
        viewports = self.getViewports()
        current = self.getActiveViewport()

        index0 = -1 if (increment > 0) else len(viewports)
        index = viewports.index(current) if (current is not None) else index0
        index += increment

        current = viewports[index] if ((index < len(viewports)) and (index >= 0)) else None

        self.setActiveViewport(current)
        self._window.render()

    def getSources(self):
        """Complete list of available ChiggerSourceBase objects"""
        return [source for viewport in self.getViewports() for source in viewport.sources() if source.getParam('pickable')]

    def getActiveSource(self):
        """Current active ChiggerSourceBase object"""
        for source in self.getSources():
            if source.getParam('interactive'):
                return source
        return None

    def setActiveSource(self, source):
        for s in self.getSources():
            active = s is source
            s.setParams(highlight=active, interactive=active)
            s._viewport.updateInformation()
            s.updateInformation()

        src = self.getActiveSource()
        if src is not None:
            src._viewport.setParams(interactive=True)
            src._viewport.updateInformation()

    def onNextSource(self, increment):
        """
        Activate the "next" source object
        """
        self.debug('Select Next Source')

        # Remove active viewport
        self.setActiveViewport(None)

        # Determine the index of the ChiggerSourceBase to be set to active
        sources = self.getSources()
        current = self.getActiveSource()

        index0 = -1 if (increment > 0) else len(sources)
        index = sources.index(current) if (current is not None) else index0
        index += increment

        current = sources[index] if ((index < len(sources)) and (index >= 0)) else None
        self.setActiveSource(current)

        self._window.render()

    def onDeactivate(self):
        """Remove all interaction seclections"""
        self.setActiveViewport(None)
        self.setActiveSource(None)

    def onPrintParams(self):
        """Print a list of all available options for active objects."""
        def printHelper(obj):
            if obj is not None:
                print(mooseutils.colorText('\n{} Available Params:'.format(obj.name()), 'LIGHT_CYAN'))
                print(obj.parameters())

        obj = self.getActiveSource() or self.getActiveViewport()
        if obj is not None:
            printHelper(obj)
        else:
            self.warning("No active viewport or source, so there is nothing to print (press 'h' for help).")

    def onPrintSetParams(self, *args):
        """Print python code for the 'setParams' method for active objects"""
        def printHelper(obj):
            if obj is not None:
                output, sub_output = obj.parameters().toScript()
                print('\n{} -> setParams({})'.format(obj.name(), ', '.join(output)))
                for key, value in sub_output.items():
                    print('{} -> setParams({}, {})'.format(obj.name(), key, ', '.join(repr(value))))

        obj = self.getActiveSource() or self.getActiveViewport()
        if obj is not None:
            printHelper(obj)
        else:
            self.warning("No active viewport or source, so there is nothing to print (press 'h' for help).")

    def onWriteChanges(self):
        """Write changes from the supplied object directly to the script, if desired"""

        # Determine the object to glean options from and error if two things are active
        obj = self.getActiveSource() or self.getActiveViewport()
        if obj is not None:
            self._onWriteChanges(obj)
        else:
            self.warning("No active source or viewport to inspect for option changes, so there is nothing to write (press 'h' for help).")

    def onPrintCamera(self):
        viewport = self.getActiveViewport()
        if viewport is None:
            source = self.getActiveSource()
            viewport = source._viewport if source is not None else None

        if viewport is not None:
            print('\n'.join(utils.print_camera(viewport.getVTKRenderer().GetActiveCamera())))
        else:
            self.warning("No active source or viewport, so the desired camera is unknown (press 'h' for help).")

    def onQuit(self):
        self._window.terminate()

    def _onKeyPressEvent(self, obj, event):
        """
        The function to be called by the vtkInteractor KeyPressEvent (see init).

        Inputs:
            obj, event: Required by VTK.
        """
        key = obj.GetKeySym().lower()
        shift = obj.GetShiftKey()
        self.debug('Key press: {}, shift={}', key, shift)

        # This objects bindings
        for binding in self.getKeyBindings(key, shift):
            binding.function(self, *binding.args)

        # Viewport options
        viewport = self.getActiveViewport()
        if viewport is not None:
            for binding in viewport.getKeyBindings(key, shift):
                binding.function(viewport, *binding.args)

        # Source options
        source = self.getActiveSource()
        if source is not None:
            for binding in source.getKeyBindings(key, shift):
                binding.function(source, *binding.args)

        self._window.render()
