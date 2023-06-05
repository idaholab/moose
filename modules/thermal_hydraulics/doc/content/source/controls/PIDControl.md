# PIDControl

!syntax description /Controls/PIDControl

The reference or target value is set by the [!param](/Controls/PIDControl/set_point) parameter control data.
The value of the `output` data is set by:

!equation
\text{error} = \text{set point} - \text{value} \\
\text{output} = K_p * \text{error} + \text{error integral} + K_d * \dfrac{\text{error} - \text{error}_{old}}{dt};

where $K_p$ and $K_d$ are constant parameters of the PID logic, the `set point` and the `value` are input control data
and the error integral is computed using from the error using a simple rectangle integration.

!alert note
To control a controllable value directly instead of a control data, use the [PIDTransientControl.md]

!syntax parameters /Controls/PIDControl

!syntax inputs /Controls/PIDControl

!syntax children /Controls/PIDControl
