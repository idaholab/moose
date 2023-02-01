# PorousFlow module

!media porous_flow/1phase_convection.gif style=width:80%;margin-left:10px caption=Density driven [convective mixing](porous_flow/lava_lamp.md) of CO2 in brine  id=1phase_convection_anim

The PorousFlow module is a library of physics for fluid and heat flow in porous
media. It is formulated in an extremely general manner, so is capable of solving
problems with an arbitrary number of phases (gas, liquid, etc) and fluid components (species present in
each fluid phase), using any set of primary variables.

By simply adding pieces of physics together in an input file, the PorousFlow
module enables the user to model problems with any combination of fluid, heat
and geomechanics.

!row!
!col! small=12 medium=4 large=4 icon=device_hub

### Module overview class=center style=font-weight:200;

- [Governing equations](porous_flow/governing_equations.md)
- [Material laws](porous_flow/material_laws.md)
- [Fluid equations of state](porous_flow/fluids.md)
- [Boundaries](porous_flow/boundaries.md)
- [Point and line sources and sinks](porous_flow/sinks.md)
- [Flow models](porous_flow/flow_models.md)
- [Additional objects (actions, postprocessors, etc)](porous_flow/additional_objects.md)
- [Full system documentation](porous_flow/systems.md)
- [A to Z Index](porous_flow/contents.md)

!col-end!

!col! small=12 medium=4 large=4 icon=school

### Tutorial and examples class=center style=font-weight:200;

!include modules/porous_flow/porous_flow_examples.md start=example-list-begin end=example-list-end

!col-end!

!col! small=12 medium=4 large=4 icon=storage

### Implementation details class=center style=font-weight:200;

- [PorousFlowDictator](porous_flow/dictator.md)
- [Numerical stabilization](porous_flow/stabilization.md)
- [Preconditioning and solvers](porous_flow/solvers.md)
- [Convergence criteria](porous_flow/convergence.md)
- [Nonlinear convergence problems](porous_flow/nonlinear_convergence_problems.md)
- [Persistent variables](porous_flow/persistent_variables.md)
- [Time derivative](porous_flow/time_derivative.md)
- [Lagrangian coordinates](porous_flow/lagrangian_eulerian.md)
- [Compositional flash](porous_flow/compositional_flash.md)

!col-end!
!row-end!

## Become a developer

The PorousFlow module is being developed by users at national laboratories
and universities around the world. Learn how to get in touch with the developers
on the [help/contact_us.md optional=True] page.

All users of PorousFlow are encouraged to assist in the development of this module. There are
a large number of possible enhancements that can be implemented, and better documentation that could
be contributed, so consider becoming a developer yourself. Follow the MOOSE standards for [contributing code](framework/contributing.md).
