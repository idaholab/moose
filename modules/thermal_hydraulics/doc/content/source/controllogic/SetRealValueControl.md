# SetRealValueControl

!syntax description /Controls/SetRealValueControl

!alert note
[ControlData.md] is only defined by the thermal hydraulics module control logic.

## Example input syntax

In this example, the `value` parameter of the `AuxKernel` called `aux_kernel`
using the `value` [ControlData.md] of the `T_inlet_fn` ControlLogic.

!listing test/tests/controls/set_real_value_control/test.i block=ControlLogic

!syntax parameters /Controls/SetRealValueControl

!syntax inputs /Controls/SetRealValueControl

!syntax children /Controls/SetRealValueControl
