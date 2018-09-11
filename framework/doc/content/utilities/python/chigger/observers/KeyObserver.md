# KeyObserver

The KeyObserver object allows users to create custom callbacks that are called when keys are
pressed within an active window object. This object is used by creating a new class that inherits
from KeyObserver and overrides the `onKeyPress(key, shift, obj, event)` method.

!listing observers/keyobserver.py
         start=import
         id=timer-example
         caption=Example script using an `KeyObserver` object to create function call triggered by a key.

!chigger options object=chigger.observers.KeyObserver

!chigger tests object=chigger.observers.KeyObserver
