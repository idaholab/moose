# ShaftConnectedMotor

This component connects to a [Shaft.md] and applies a torque $\tau$ and moment
of inertia $I$ from user-supplied functions. While this is named as a "motor"
component, its applications are more generic; for example, it can be used to
apply friction or other losses (such as the work applied to a generator) by
applying negative torque.

## Usage

The parameters [!param](/Components/ShaftConnectedMotor/torque) and
[!param](/Components/ShaftConnectedMotor/inertia) take the names of
[Functions](Functions/index.md). The shaft speed $\omega$ is used in place of
the time variable $t$ in these functions (and the space variables are
discarded); therefore, the user is actually specifying the functions
$\tau(\omega)$ and $I(\omega)$, not $\tau(t,x,y,z)$ and $I(t,x,y,z)$.

If dependence on time is desired, the [ControlLogic/index.md] can be used. To
do this, supply an arbitrary constant value instead of a function name. Then
use a [TimeFunctionComponentControl.md]. See the `motor` component in
[open_brayton_cycle.i](test/tests/problems/brayton_cycle/open_brayton_cycle.i)
for an example.

!syntax parameters /Components/ShaftConnectedMotor

!syntax inputs /Components/ShaftConnectedMotor

!syntax children /Components/ShaftConnectedMotor
