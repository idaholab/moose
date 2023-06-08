# SetComponentBoolValueControl

!syntax description /Controls/SetComponentBoolValueControl

!alert note
[ControlData.md] is only defined by the thermal hydraulics module control logic.

## Example input syntax

In this example, the `on` parameter of the `turbine` component
using the `state` [ControlData.md] of the `trip_ctrl` ControlLogic.

!listing test/tests/controls/set_component_bool_value_control/test.i block=Components ControlLogic

!syntax parameters /Controls/SetComponentBoolValueControl

!syntax inputs /Controls/SetComponentBoolValueControl

!syntax children /Controls/SetComponentBoolValueControl
