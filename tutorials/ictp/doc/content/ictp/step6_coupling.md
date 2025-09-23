# Step 6: Coupling id=ictp_step6

!---

## Coupling Problem

Next, we will couple our fluid and solid (fuel pin) systems together at the interface between the two meshes (along the `water_solid_interface` boundary in each problem).

This aims to be an example of multiphysics coupling within MOOSE. Note that it isn't a true multiphysics simulation, but it is an example on the use of the [`MultiApp`](MultiApps/index.md) system and the [`Transfer`](Transfers/index.md) system.

The two inputs that were created in [#ictp_step5] will be extended in this problem.

!---

## Problem Definition

!style halign=center
!media step6_setup.png style=width:50%

!---

## `MultiApp` System

MOOSE was originally created to solve fully-coupled systems of [!ac](PDEs). Systems of [!ac](PDEs) in MOOSE can be solved either tightly-coupled or loosely-coupled:

- Tight coupling can be accomplished by solving multiple [!ac](PDEs) in a single matrix system, or via operator-splitting plus fixed point iteration
- Multiscale systems are generally loosely coupled between scales
- Systems with both fast and slow physics can be decoupled in time

A `MultiApp` allows you to simultaneously solve for individual physics systems.

You can think of a `MultiApp` of an instantiation (or many instantiations!) of another input file that can be executed during the main application's solve.

!---

## `MultiApp` Input Example

```moose
[MultiApps/fluid]
  type = TransientMultiApp
  input_files = 'fluid.i'
  execute_on = TIMESTEP_END
[]
```

!---

## `Transfer` System

A [`Transfer`](Transfers/index.md) describes how you move information up and down the MultiApp hierarchy.

There are three common ways a `Transfer` can read information and deposit information:

- `Variable` (read only) and `AuxVariable` fields
- `PostProcessor` values
- `UserObject` information (not discussed here)

+By having MOOSE move data and fill fields... apps don't need to know or care where the data came from or how it got there!+

!---

## `Transfer` Input Example

```moose
[Transfers/send_heat_flux]
  type = MultiAppGeneralFieldNearestLocationTransfer
  from_multi_app = fluid
  source_variable = T_fluid
  variable = T_fluid
  to_boundaries = water_solid_interface
[]
```

!---

## Problem Definition

!style halign=center
!media step6_setup.png style=width:50%

!---

## Problem `MultiApp` Summary

We will have two input files: `solid.i` and `fluid.i`. The solid (fuel pin) input will the "main" input, thus the fluid input will have a `MultiApp`.

This will require a single `[MultiApp]` in `solid.i` (for `fluid.i`) and two transfers in `solid.i`:

- The transfer of the heat flux from the solid to the fluid on the periphery of the clad (the `water_solid_interface` boundary)
- The transfer of the fluid temperature on its inner interface (also `water_solid_interface`) to the solid

!---

## Problem `MultiApp` Block

!listing ictp/inputs/step6_coupling/solid.i block=MultiApps prefix=moose/step6_coupling

!---

## Problem `Transfers` Block

!listing ictp/inputs/step6_coupling/solid.i block=Transfers prefix=moose/step6_coupling

!---

## Problem Input: Solid

!listing ictp/inputs/step6_coupling/solid.i diff=ictp/inputs/step5_heat_conduction/solid.i prefix=moose/step6_coupling diff_prefix=moose/step5_heat_conduction

!---

## Problem Input: Fluid

!listing ictp/inputs/step6_coupling/fluid.i diff=ictp/inputs/step5_heat_conduction/fluid.i prefix=moose/step6_coupling diff_prefix=moose/step5_heat_conduction

!---

# Run: Coupled Problem

```bash
$ cd ../step6_coupling
$ cardinal-opt -i solid.i
```

!--

# Result: Coupled Problem $T$, Last Timestep

!style halign=center
!media step6_solution.png style=width:50%

!style halign=center
From `solid_out.e` and `solid_out_fluid0.e` in Paraview, last timestep
