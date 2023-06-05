# SetComponentRealValueControl

!syntax description /Controls/SetComponentRealValueControl

!alert note
Control data is only defined by the thermal hydraulics module control logic.

## Example input syntax

In this example, the `T0` parameter of the `inlet` component
using the `value` control data of the `T_inlet_fn` ControlLogic.

!listing test/tests/controls/set_component_real_value_control/test.i block=Components ControlLogic

!syntax parameters /Controls/SetComponentRealValueControl

!syntax inputs /Controls/SetComponentRealValueControl

!syntax children /Controls/SetComponentRealValueControl
