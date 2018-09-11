# SingleShotObserver

The SingleShotObserver object allows users to create custom callbacks that are called once based
on a timer duration within an active window object. This object is used by creating a new class that
inherits from SingleShotObserver and overrides the `onTimer(obj, event)` method.

!chigger options object=chigger.observers.SingleShotObserver

!chigger tests object=chigger.observers.SingleShotObserver
