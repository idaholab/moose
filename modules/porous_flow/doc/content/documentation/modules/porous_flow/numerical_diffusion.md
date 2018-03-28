# Numerical diffusion

Numerical diffusion has been mentioned in various other pages.  For example, it is evident in the sinks and newton cooling tests.  There are two main sources of numerical diffusion:

- employing large time steps with MOOSE's implicit time stepping scheme
- the full upwinding used by PorousFlow

(One current area of active development of the PorousFlow code is the use of reconstructed discontinuous Galerkin methods to reduce numerical diffusion.)

To quantify the numerical by example, a single-phase tracer advection problem is studied in 1D.  A porepressure gradient is established so that the Darcy velocity, $k \nabla P/\mu = 10^{-2}\,$m/s, and porosity is chosen to be $0.1$ so that the tracer advects down the porepressure gradient with a constant velocity of $\phi k\nabla P/\mu = 0.1\,$m/s.

Two input files are created: one using the fully-saturated action, which is mass-lumped but does not use any upwinding; and the other that uses the standard mass-lumped and fully-upwinded PorousFlow kernels.  They are:

!listing modules/porous_flow/test/tests/numerical_diffusion/fully_saturated_action.i

!listing modules/porous_flow/test/tests/numerical_diffusion/no_action.i

An animation of the tracer advection is shown in [tracer_advection_anim].  Notice that the initially-sharp profile suffers from diffusion.

!media porous_flow/tracer_advection.gif style=width:50%;margin-left:10px caption=Tracer advection down the porepressure gradient.  id=tracer_advection_anim

The degree of diffusion is a function of the spatial and temporal discretisation, as well as of the upwinding.  [upwind_eles] and [upwind_dt] show the dependence on discretisation when full upwinding is used.

!media media/porous_flow/upwind_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and full upwinding is used.  id=upwind_eles

!media media/porous_flow/upwind_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and full upwinding is used.  id=upwind_dt

[no_upwind_eles] and [no_upwind_dt] show the dependence on discretisation when there is no upwinding.  Evidently, the lack of upwinding causes overshoots and undershoots, and the diffusion becomes less as the spatial discretisation becomes finer.

!media media/porous_flow/no_upwind_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and no upwinding is used.  id=no_upwind_eles

!media media/porous_flow/no_upwind_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and no upwinding is used.  id=no_upwind_dt

The diffusive nature of the full upwinding is revealed by comparing the case where there are 100 elements and 100 timesteps ([upwind_eles] with [no_upwind_eles]).






