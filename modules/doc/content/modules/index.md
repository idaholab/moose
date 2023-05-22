# Physics Modules

MOOSE includes a set of community developed physics modules that you can build on to create your own
application.

- [Chemical reactions](chemical_reactions:modules/chemical_reactions/index.md)
- [Combined](combined:modules/combined/index.md)
- [Contact](contact:modules/contact/index.md)
- [Electromagnetics](electromagnetics:modules/electromagnetics/index.md)
- [Fluid Properties](fluid_properties:modules/fluid_properties/index.md)
- [Fluid-Structure Interaction](fsi:modules/fsi/index.md)
- [Functional Expansion Tools](functional_expansion_tools:modules/functional_expansion_tools/index.md)
- [Geochemistry](geochemistry:modules/geochemistry/index.md)
- [Heat Conduction](heat_conduction:modules/heat_conduction/index.md)
- [Level Set](level_set:modules/level_set/index.md)
- [Misc](misc:modules/misc/index.md)
- [Navier-Stokes](navier_stokes:modules/navier_stokes/index.md)
- [Optimization](optimization:modules/optimization/index.md)
- [Peridynamics](peridynamics:modules/peridynamics/index.md)
- [Phase Field](phase_field:modules/phase_field/index.md)
- [Porous Flow](porous_flow:modules/porous_flow/index.md)
- [Ray Tracing](ray_tracing:modules/ray_tracing/index.md)
- [Reactor](reactor:modules/reactor/index.md)
- [Reconstructed Discontinous Galerkin](rdg:modules/rdg/index.md)
- [Richards](richards:modules/richards/index.md)
- [Solid Properties](solid_properties:modules/solid_properties/index.md)
- [Stochastic Tools](stochastic_tools:modules/stochastic_tools/index.md)
- [Tensor Mechanics](tensor_mechanics:modules/tensor_mechanics/index.md)
- [Thermal Hydraulics](thermal_hydraulics:modules/thermal_hydraulics/index.md)
- [XFEM](xfem:modules/xfem/index.md)

The purpose of the modules is to encapsulate common kernels, boundary conditions, etc. to prevent
code duplication.  Examples include: heat conduction, solid mechanics, Navier-Stokes, and others. The
modules are organized so that your application can link against only those which it requires.

!alert warning
No export controlled physics (e.g., neutronics) should be put into the modules.

## Combined Examples

Below contains a list of examples that utilize a combination of the physics modules listed above.

- [combined:modules/combined/examples/stm_thermomechanics.md]
