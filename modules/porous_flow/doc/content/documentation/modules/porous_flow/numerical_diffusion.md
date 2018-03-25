# Numerical diffusion

Numerical diffusion has been mentioned in various other pages.  For example, it is evident in the sinks and newton cooling tests.  There are two main sources of numerical diffusion:

- employing large time steps with MOOSE's implicit time stepping scheme
- the full upwinding

To quantify this by example, a single-phase tracer advection problem is studied in 1D.  A porepressure gradient is established so that the Darcy velocity, $k \nabla P/\mu = 10^{-2}\,$m/s, and porosity is chosen to be $0.1$ so that the tracer advects down the porepressure gradient with velocity is $0.1\,$m/s.  Two input files are created: one using the fully-saturated action, which is mass-lumped but does not use any upwinding; and the other that uses the standard mass-lumped and fully-upwinded PorousFlow kernels.  They are:

!listing modules/porous_flow/test/tests/numerical_diffusion/fully_saturated_action.i

!listing modules/porous_flow/test/tests/numerical_diffusion/no_action.i

An animation of the tracer advection is shown in [tracer_advection_anim].  Notice that the initially-sharp profile suffers from diffusion.
n
!media porous_flow/tracer_advection.gif style=width:50%;margin-left:10px caption=Tracer advection down the porepressure gradient.  id=tracer_advection_anim

The degree of diffusion is a function of the spatial and temporal discretisation, as well as of the upwinding.  [no_upwind_dt] and [no_upwind_eles] show the dependence on discretisation when there is no upwinding.  Evidently, the lack of upwinding causes overshoots and undershoots, and the diffusion becomes less as the spatial discretisation becomes finer.  [upwind_dt] and [upwind_eles] show the dependence on discretisation when full upwinding is used: there are no overshoots but there is typically more diffusion than when using no upwinding.

!media media/porous_flow/no_upwind_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and no upwinding is used.  id=no_upwind_dt

!media media/porous_flow/no_upwind_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and no upwinding is used.  id=no_upwind_eles

!media media/porous_flow/upwind_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and full upwinding is used.  id=upwind_dt

!media media/porous_flow/upwind_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and full upwinding is used.  id=upwind_eles






