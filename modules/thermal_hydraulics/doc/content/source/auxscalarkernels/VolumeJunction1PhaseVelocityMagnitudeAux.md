# VolumeJunction1PhaseVelocityMagnitudeAux

!syntax description /AuxScalarKernels/VolumeJunction1PhaseVelocityMagnitudeAux

The velocity magnitude is computed as

!equation
\|\mathbf{u}\| = \left\|\left(\dfrac{\rho u V}{\rho V}, \dfrac{\rho v V}{\rho V}, \dfrac{\rho w V}{\rho V}\right)\right\|

where $\rho$ is the density of the single phase fluid at the junction, $\mathbf{u}=(u, v, w)$ is the fluid velocity vector,
and $V$ is the volume of the junction.

This object is automatically added to the simulation by the [VolumeJunction1Phase.md] component.

!syntax parameters /AuxScalarKernels/VolumeJunction1PhaseVelocityMagnitudeAux

!syntax inputs /AuxScalarKernels/VolumeJunction1PhaseVelocityMagnitudeAux

!syntax children /AuxScalarKernels/VolumeJunction1PhaseVelocityMagnitudeAux
