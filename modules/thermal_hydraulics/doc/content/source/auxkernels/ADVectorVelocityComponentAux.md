# ADVectorVelocityComponentAux

!syntax description /AuxKernels/ADVectorVelocityComponentAux

The direction is of the velocity is obtained from providing a component index into the
[!param](/AuxKernels/ADVectorVelocityComponentAux/direction) material property parameter.

The velocity component is computed as:

!equation
velocity = direction(component) * \dfrac{\alpha * \rho * u * A}{\alpha * \rho * A}

where $\alpha$ is the phase fraction, $\rho$ the density, $A$ the local area and $u$ the 1D velocity.

!alert note
This auxkernel may not be used with a local zero phase fraction.

!alert note
$\rho u A$, $\rho A$ and $\alpha$ are variables usually defined by the [Components](syntax/Components/index.md).

!syntax parameters /AuxKernels/ADVectorVelocityComponentAux

!syntax inputs /AuxKernels/ADVectorVelocityComponentAux

!syntax children /AuxKernels/ADVectorVelocityComponentAux
