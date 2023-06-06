# TerminateControl

!syntax description /Controls/TerminateControl

The boolean that determines when to terminate is a [ControlData.md]. This is only used in the
THM control logic to keep track of states of control objects.

The boolean can be examined in the console using a [BoolControlDataValuePostprocessor.md].

!alert note
Another way to control when to terminate a simulation is to use the [Terminator.md] user object.
The [Terminator.md] works with postprocessors for the criterion instead of a controlled boolean.

## Example input

In this example, the `TerminateControl` examines the state of the `UnitTripControl`. Once its state is nonzero, it
stop the simulation and prints `Threshold exceeded`.

!listing test/tests/controls/terminate/terminate.i block=Postprocessors ControlLogic

!syntax parameters /Controls/TerminateControl

!syntax inputs /Controls/TerminateControl

!syntax children /Controls/TerminateControl
