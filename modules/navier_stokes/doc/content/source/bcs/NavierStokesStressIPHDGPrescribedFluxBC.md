# NavierStokesStressIPHDGPrescribedFluxBC

This class weakly imposes a prescribed traction vector $\sigma\hat{n}$. Perhaps the most common
use case for this boundary condition is for a zero traction condition
at outflow boundaries, as shown in the MMS channel case

!listing modules/navier_stokes/test/tests/finite_element/ins/hdg/ip/channel-flow/mms-channel.i block=BCs/momentum_x_diffusion_neumann

and

!listing modules/navier_stokes/test/tests/finite_element/ins/hdg/ip/channel-flow/mms-channel.i block=BCs/momentum_y_diffusion_neumann

When applying a zero traction condition at an outflow, if the flow is fully developed, then
implicitly this applies a zero pressure condition.

!syntax parameters /BCs/NavierStokesStressIPHDGPrescribedFluxBC

!syntax inputs /BCs/NavierStokesStressIPHDGPrescribedFluxBC

!syntax children /BCs/NavierStokesStressIPHDGPrescribedFluxBC
