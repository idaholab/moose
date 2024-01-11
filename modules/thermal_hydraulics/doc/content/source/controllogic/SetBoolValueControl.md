# SetBoolValueControl

!syntax description /ControlLogic/SetBoolValueControl

!alert note
[ControlData.md] is only defined by the thermal hydraulics module control logic.

## Example input syntax

In this example, the `value` parameter of the `AuxKernel` called `aux_kernel`
using the `state` [ControlData.md] of the `threshold_ctrl` ControlLogic.

!listing test/tests/controls/set_bool_value_control/test.i block=ControlLogic

!syntax parameters /ControlLogic/SetBoolValueControl

!syntax inputs /ControlLogic/SetBoolValueControl

!syntax children /ControlLogic/SetBoolValueControl
