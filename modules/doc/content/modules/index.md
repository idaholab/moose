# Physics Modules

MOOSE includes a set of community developed physics modules that you can build on to create your own
application.

- [Chemical reactions](modules/chemical_reactions/index.md)
- [Contact](modules/contact/index.md)
- [Fluid Properties](modules/fluid_properties/index.md)
- [Functional Expansion Tools](modules/functional_expansion_tools/index.md)
- [Geochemistry](modules/geochemistry/index.md)
- [Heat Conduction](modules/heat_conduction/index.md)
- [Level Set](modules/level_set/index.md)
- [Misc](modules/misc/index.md)
- [Navier-Stokes](modules/navier_stokes/index.md)
- [Peridynamics](modules/peridynamics/index.md)
- [Phase Field](modules/phase_field/index.md)
- [Porous Flow](modules/porous_flow/index.md)
- [Reconstructed Discontinous Galerkin](modules/rdg/index.md)
- [Richards](modules/richards/index.md)
- [Stochastic Tools](modules/stochastic_tools/index.md)
- [Tensor Mechanics](modules/tensor_mechanics/index.md)
- [XFEM](modules/xfem/index.md)

The purpose of the modules is to encapsulate common kernels, boundary conditions, etc. to prevent
code duplication.  Examples include: heat conduction, solid mechanics, Navier-Stokes, and others. The
modules are organized so that your application can link against only those which it requires.

!alert warning
No export controlled physics (e.g., neutronics) should be put into the modules.

## Combined Examples

Below contains a list of examples that utilize a combination of the physics modules listed above.

- [modules/combined/examples/stm_thermomechanics.md]
