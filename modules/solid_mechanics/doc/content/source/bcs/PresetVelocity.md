# PresetVelocity

!syntax description /BCs/PresetVelocity

This Dirichlet boundary condition lets the user set the displacement using the [!param](/BCs/PresetVelocity/function) and [!param](/BCs/PresetVelocity/velocity) parameters.

The displacement is updated as:

!equation
u(t) = u(t-dt) + v \dfrac{f(t) + f(t-dt)}{2}

where $u$ is the displacement, $f$ the [!param](/BCs/PresetVelocity/function) parameter and $v$ the [!param](/BCs/PresetVelocity/velocity) parameter, $t$ the current time and dt the time step.

!alert warning
This boundary condition hard-codes the use of a first order Euler time integration scheme.

!syntax parameters /BCs/PresetVelocity

!syntax inputs /BCs/PresetVelocity

!syntax children /BCs/PresetVelocity