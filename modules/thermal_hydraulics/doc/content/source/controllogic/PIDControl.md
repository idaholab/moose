# PIDControl

!syntax description /ControlLogic/PIDControl

The reference or target value is set by the [!param](/ControlLogic/PIDControl/set_point) parameter [ControlData.md].
The value of the `output` data is set by:

!equation
\text{error} = \text{set point} - \text{value} \\
\text{error integral} = \text{error integral} + K_i * \text{error} * dt \\
\text{output} = K_p * \text{error} + \text{error integral} + K_d * \dfrac{\text{error} - \text{error}_{old}}{dt};

where $K_p$, $K_i$ and $K_d$ are the proportional, integral and derivative constant parameters of the PID logic,
the `set point` and the `value` are user-selected [ControlData.md].

!alert note
To control a controllable value directly instead of a [ControlData.md], use the [PIDTransientControl.md]

!syntax parameters /ControlLogic/PIDControl

!syntax inputs /ControlLogic/PIDControl

!syntax children /ControlLogic/PIDControl
