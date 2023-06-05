# UnitTripControl

!syntax description /Controls/UnitTripControl

This object defines a control logic boolean `state` that is set to true if the
condition is true, and false if the condition is false.
Using the [!param](/Controls/UnitTripControl/latch) parameter, the `state` can always remain true
after a trip, or go back to false if the expression no longer evaluates to true.

## Example input syntax

In this example, the value of the postprocessor `a` controls the trip of the ControlLogic `trip_ctrl`,
when its value becomes larger than 0.6.

!listing test/tests/controls/unit_trip_control/test.i block=Postprocessors ControlLogic

!syntax parameters /Controls/UnitTripControl

!syntax inputs /Controls/UnitTripControl

!syntax children /Controls/UnitTripControl
