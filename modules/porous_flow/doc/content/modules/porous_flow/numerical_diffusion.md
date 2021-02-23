# Numerical diffusion

This page is part of a set of pages devoted to discussions of numerical stabilization in PorousFlow.  See:

- [Numerical stabilization lead page](stabilization.md)
- [Mass lumping](mass_lumping.md)
- [Full upwinding](upwinding.md)
- [Kuzmin-Turek stabilization](kt.md)
- [Numerical diffusion](numerical_diffusion.md)
- [A worked example of Kuzmin-Turek stabilization](kt_worked.md)

Numerical diffusion is the artifical smoothing of quantities, such as temperature and concentrations, as they are transported through a numerical model.  In the case of PorousFlow, it is usually the fluid that transports these quantities (the fluid "advects" temperature and chemical species).  Numerical diffusion can be the major source of inaccurate results in simulations, as MOOSE predicts that tracer breakthrough times (etc) are much shorter than they are in reality.

An animation of the tracer advection is shown in [tracer_advection_anim].  Notice that the initially-sharp profile suffers from diffusion.

!media porous_flow/tracer_advection.gif style=width:50%;margin-left:10px caption=Tracer advection down the porepressure gradient.  id=tracer_advection_anim

Numerical diffusion has been mentioned in various pieces of PorousFlow documentation (eg, in the [tutorial_06.md] and [source/kernels/PorousFlowFullySaturatedDarcyFlow.md], and the documentation concerning the [sinks tests](porous_flow/tests/sinks/sinks_tests.md) and [heat advection test page](porous_flow/tests/heat_advection/heat_advection_tests.md)).  There are two main sources of numerical diffusion:

- employing large time steps with MOOSE's implicit time stepping scheme
- the full upwinding used by PorousFlow

To quantify the numerical diffusion by example, a single-phase tracer advection problem is studied in 1D.  A porepressure gradient is established so that the Darcy velocity, $k \nabla P/\mu = 10^{-2}\,$m/s, and porosity is chosen to be $0.1$ so that the tracer advects down the porepressure gradient with a constant velocity of $\phi k\nabla P/\mu = 0.1\,$m/s.

The degree of numerical diffusion is a function of the spatial and temporal discretisation, as well as the upwinding, RDG reconstruction and limiting.  To quantify this, seven input files are created:

1. Using framework MOOSE objects: there is no mass lumping and no upwinding.
2. Using the PorousFlow fully-saturated action, which is mass-lumped but does not use any upwinding.
3. Using the standard mass-lumped and fully-upwinded PorousFlow kernels.
4. Using the RDG module with no reconstruction (RDG(P0)): the tracer is a constant monomial Variable (c.f. linear lagrange variables in PorousFlow)
5. Using the RDG module with linear reconstruction (RDG(P0P1)): the tracer is constant monomial, but a linear form is reconstructed and then limited using RDG's mc limiter .  Another limiter could have been chosen, with very similar results.
6. Using the Kuzmin-Turek scheme with no flux limiter (should be identical to the fully upwinded case).
7. Using the Kuzmin-Turek scheme with a flux limiter (should be similar to RDG(POP1) case).

The Kuzmin-Turek scheme is described in: D Kuzmin and S Turek (2004) "High-resolution FEM-TVD schemes based on a fully multidimensional flux limiter." Journal of Computational Physics, Volume 198, 131 - 158.

## No mass lumping and no upwinding

The MOOSE input file is:

!listing modules/porous_flow/test/tests/numerical_diffusion/framework.i

[framework_eles] and [framework_dt] show the dependence on discretisation when there is no mass-lumping and no upwinding.  Evidently, the lack of upwinding causes overshoots and undershoots, and the diffusion becomes less as the spatial discretisation becomes finer.

!media media/porous_flow/framework_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and no mass lumping and no upwinding is used.  id=framework_eles

!media media/porous_flow/framework_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and no mass lumping and no upwinding is used.  id=framework_dt


## Mass lumping and no upwinding

The MOOSE input file is:

!listing modules/porous_flow/test/tests/numerical_diffusion/fully_saturated_action.i

[no_upwind_eles] and [no_upwind_dt] show the dependence on discretisation when there is no upwinding.  Evidently, the lack of upwinding causes overshoots and undershoots, and the diffusion becomes less as the spatial discretisation becomes finer.

!media media/porous_flow/no_upwind_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and no upwinding is used.  id=no_upwind_eles

!media media/porous_flow/no_upwind_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and no upwinding is used.  id=no_upwind_dt


## Mass lumping and full upwinding

The MOOSE input file is identical to the one above, save for the `PorousFlowFullySaturated` block that must now contain `stabilization = Full`:

```
[PorousFlowFullySaturated]
  porepressure = porepressure
  coupling_type = Hydro
  gravity = '0 0 0'
  fp = the_simple_fluid
  mass_fraction_vars = tracer
  stabilization = Full
[]
```

(Also, the test suite contains a [non-action version](https://github.com/idaholab/moose/blob/master/modules/porous_flow/test/tests/numerical_diffusion/no_action.i)).

[upwind_eles] and [upwind_dt] show the dependence on discretisation when full upwinding is used.

!media media/porous_flow/upwind_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and full upwinding is used.  id=upwind_eles

!media media/porous_flow/upwind_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and full upwinding is used.  id=upwind_dt


## RDG(P0)

In RDG, the variables (just the tracer in this case) are constant-monomial, that is, they are constant throughout the element.  This means MOOSE behaves like a Finite Volume Method, and it means that the output looks less smooth than the usual linear-Lagrange variables (the figures below display "stepped" results).  The MOOSE input file is:

!listing modules/rdg/test/tests/advection_1d/rdgP0.i

[rdgP0_eles] and [rdgP0_dt] show the dependence on discretisation when RDG(P0) is used.  This is identical to mass-lumping + full-upwinding (up to the fact that constant-monomial variables are used instead of linear-Lagrange).  As expected, there are no oscillations or over-shoots or under-shoots, but the results suffer from numerical diffusion.

!media media/porous_flow/rdgP0_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and RDG(P0) is used.  id=rdgP0_eles

!media media/porous_flow/rdgP0_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and RDG(P0) is used.  id=rdgP0_dt


## RDG(P0P1)

The MOOSE input file is almost identical to the RDG(P0) version, save for the changing the SlopeLimiting scheme from "none" to "mc" (or another limiter such as "minmod" or "superbee").

[rdgP0P1_eles] and [rdgP0P1_dt] show the dependence on discretisation when RDG(P0P1) is used.  As with RDG(P0), there are no oscillations or over-shoots or under-shoots.  However, numerical diffusion is greatly reduced by the linear reconstruction.

!media media/porous_flow/rdgP0P1_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and RDG(P0P1) is used.  id=rdgP0P1_eles

!media media/porous_flow/rdgP0P1_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and RDG(P0P1) is used.  id=rdgP0P1_dt

## Kuzmin-Turek scheme with no flux limiter

To employ the Kuzmin-Turek scheme, the `fully_saturated_action.i` input file listed above may be simply modified by including KT stabilization:

```
[PorousFlowFullySaturated]
  porepressure = porepressure
  coupling_type = Hydro
  gravity = '0 0 0'
  fp = the_simple_fluid
  mass_fraction_vars = tracer
  stabilization = KT
  flux_limiter_type = None
[]
```

in the `PorousFlowFullySaturated` Action block.  Alternatively, a new input file may be built, such as:

!listing modules/porous_flow/test/tests/numerical_diffusion/fltvd_no_antidiffusion.i

[KTnone_eles] and [KTnone_dt] show the dependence on discretisation when Kuzmin-Turek with no flux limiter is used. As expected, the behaviour is identical to the case with full upwinding.

!media media/porous_flow/KTnone_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and Kuzmin-Turek with no flux limiter is used.  id=KTnone_eles

!media media/porous_flow/KTnone_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and Kuzmin-Turek with no flux limiter is used.  id=KTnone_dt

## Kuzmin-Turek scheme with flux limiter

To employ the Kuzmin-Turek scheme, the `fully_saturated_action.i` input file listed above may be simply modified by including KT stabilization:

```
[PorousFlowFullySaturated]
  porepressure = porepressure
  coupling_type = Hydro
  gravity = '0 0 0'
  fp = the_simple_fluid
  mass_fraction_vars = tracer
  stabilization = KT
  flux_limiter_type = superbee
[]
```

in the `PorousFlowFullySaturated` Action block.  Alternatively, a new input file may be built, such as:

!listing modules/porous_flow/test/tests/numerical_diffusion/fltvd.i

[KTlimited_eles] and [KTlimited_dt] show the dependence on discretisation when Kuzmin-Turek with a flux limiter is used (the "superbee" limiter has been chosen in this case, see the [worked example of Kuzmin-Turek stabilization](kt_worked.md)). As expected, the behaviour is similar to the RDG(POP1) case, except it is smooth because the variables are linear-lagrange.

!media media/porous_flow/KTlimited_eles.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of elements.  The number of time steps is fixed to 100 and Kuzmin-Turek with the superbee flux limiter.  id=KTlimited_eles

!media media/porous_flow/KTlimited_dt.png style=width:60%;margin-left:10px caption=Diffusion as a function of number of time steps.  The number of elements is fixed to 100 and Kuzmin-Turek with the superbee flux limiter.  id=KTlimited_dt

## Summary

By way of comparison, [nds_100_100] and [nds_100_1000] show the results when the number of elements is fixed to 100 and the number of timesteps is 100 or 1000 (respectively).  Evidently, without upwinding, the result includes over-shoots and under-shoots, which are typically devastating for PorousFlow simulations, since they typically manifest themselves as negative saturations, negative mass fractions, or negative temperatures.  With upwinding, RDG(P0), or Kuzmin-Turek with no antidiffusion these are avoided, at the expense of introducing large numerical diffusion.  RDG(P0P1) and Kuzmin-Turek with a flux limiter produces a similar amount of numerical diffusion to the non-upwinded, non-masslumped version, but without introducing any over-shoots and under-shoots.

!media media/porous_flow/numerical_diffusion_summary_100_100.png style=width:60%;margin-left:10px caption=Diffusion for different numerical schemes.  100 timesteps are used.  id=nds_100_100

!media media/porous_flow/numerical_diffusion_summary_100_1000.png style=width:60%;margin-left:10px caption=Diffusion for different numerical schemes.  1000 timesteps are used.  id=nds_100_1000

Interestingly, as the timestep size is decreased, the numerical diffusion is decreased when using lumping and upwinding, but only up to a point: increasing the number of timesteps from 100 to 10000 makes basically no difference when the number of elements is 100.  This suggests that lumping and full-upwinding will always produce numerical diffusion, and this may be proved by simple analysis (not presented here) since the tracer always gets moved from upwind node to downwind node irrespective of the concentration of the tracer.





