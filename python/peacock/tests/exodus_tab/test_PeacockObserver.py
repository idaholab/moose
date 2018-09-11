#!/usr/bin/env python
import chigger
from peacock.utils import Testing
from peacock.ExodusViewer.plugins.PeacockObserver import PeacockObserver

class Test(chigger.observers.TestObserver):
    def onTimer(self, *args):
        self.moveMouse(0.81, 0.26)
        self.pressLeftMouseButton()

        self.moveMouse(0.11, 0.26)

        # The callback doesn't get triggered for some reason, but we can fake it for testing.
        observer = self._window.getOption('default_observer')
        observer._onMouseMoveEvent(self._window.getVTKInteractor(), None)
        self._window.write('PeacockObserver.png')

reader = chigger.exodus.ExodusReader(Testing.get_chigger_input('mug_blocks_out.e'))
result = chigger.exodus.ExodusResult(reader,
                                     variable='convected',
                                     cmap='shock',
                                     highlight_active=False)
cbar = chigger.misc.ColorBar(highlight_active=False)
window = chigger.RenderWindow(result, cbar,
                              default_observer=PeacockObserver(),
                              observers=[Test(terminate=True)],
                              size=(500,300))
window.start()
