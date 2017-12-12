# Physics Modules

MOOSE includes a set of community developed physics modules that you can build on to create your own application.

- [Phase Field](modules/phase_field/index.md)
- [Tensor Mechanics](modules/tensor_mechanics/index.md)
- [Reconstructed Discontinous Galerkin](modules/rdg/index.md)
- [Navier-Stokes](modules/navier_stokes/index.md)
- [Level Set](modules/level_set/index.md)
- [Fluid Properties](modules/fluid_properties/index.md)
- [Stochastic Tools](modules/stochastic_tools/index.md)
- [Porous Flow](modules/porous_flow/index.md)
- [Chemical reactions](modules/chemical_reactions/index.md)

The purpose of the modules is to encapsulate common kernels, boundary conditions, etc. to prevent code duplication.
Examples include: heat conduction, solid mechanics, Navier-Stokes, and others. The modules are organized so that your
application can link against only those which it requires.

!!! warning
    _No_ export controlled physics (e.g., neutronics) should be put into the modules.

## Modules Anatomy
---

All applications are set up to use the modules.
```bash
$ ls moose/modules
contact  misc  phase_field solid_mechanics  chemical_reactions heat_conduction  modules.mk
richards  tensor_mechanics  combined  linear_elasticity  navier_stokes  run_tests  water_steam_eos
```

The contents of each module are the same as any application.

```bash
$ ls moose/modules/solid_mechanics
Makefile  doc  include  lib  plugins  run_tests  src  tests
```

Application codes specify the modules they want to use in their Makefiles.

```make
################################## MODULES ####################################
SOLID_MECHANICS   := yes
LINEAR_ELASTICITY := yes
```


## Modules: Tensor Mechanics Example
---

Available in: [moose/modules/tensor_mechanics](https://github.com/idaholab/moose/tree/devel/modules/tensor_mechanics)

- Stats:
    - 127,650 elements, 25,227 nodes
  
- Features:
    - Large displacements
    - Plasticity and Creep

!media media/solid_mechanics/bridge.png width=32% float=right caption=[bridge]

!media media/solid_mechanics/bridge-displacement.png width=32.5% float=right padding-left=.3% padding-right=.3% caption=[displacement]

!media media/solid_mechanics/bridge-von-mises.png width=32% float=right caption=[von mises]


## Modules: Flow Example
---

Available in: [moose/modules/navier_stokes](https://github.com/idaholab/moose/tree/devel/modules/navier_stokes)

- Subsonic Test Case:
    - Mach 0.5 over a circular arc
    - Euler equations
    - 8,232 elements, 9,675 nodes

!media media/large_media/solid_mechanics/flow-velocity.png width=32% float=right caption=[velocity]

!media media/solid_mechanics/flow-streamlines.png width=32.5% float=right padding-left=.3% padding-right=.3% caption=[streamlines]

!media media/solid_mechanics/flow-pressure.png width=32% float=right caption=[pressure]
