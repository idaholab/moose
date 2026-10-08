# SetComponentRealValueControl

!syntax description /ControlLogic/SetComponentRealValueControl

!alert note
[ControlData.md] is only defined by the thermal hydraulics module control logic.

## Example input syntax

In this example, the `value` parameter of the `test_comp` component is set
from the `value` [ControlData.md] of the `T_inlet_fn` ControlLogic.

!listing test/tests/controls/set_component_real_value_control/test.i block=Components ControlLogic

!syntax parameters /ControlLogic/SetComponentRealValueControl

!syntax inputs /ControlLogic/SetComponentRealValueControl

!syntax children /ControlLogic/SetComponentRealValueControl
