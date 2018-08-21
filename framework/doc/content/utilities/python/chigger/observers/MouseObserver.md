# MouseObserver

The MouseObserver object allows users to create custom callbacks that are called when mouse is moved
within an active window object. This object is used by creating a new class that inherits
from KeyObserver and overrides the `onMouseMove(pos, obj, event)` method.

!listing observers/mouseobserver.py
         start=import
         id=timer-example
         caption=Example script using an `MouseObserver` object to create function call triggered by mouse movement.

!chigger options object=chigger.observers.MouseObserver

!chigger tests object=chigger.observers.MouseObserver
