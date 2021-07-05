# PolycrystalVoronoiCoupledVoidICAction

!syntax description /ICs/PolycrystalICs/PolycrystalVoronoiCoupledVoidIC/PolycrystalVoronoiCoupledVoidICAction

## Overview

The PolycrystalVoronoiCoupledVoidICAction generates the initial condition [PolycrystalVoronoiCoupledVoidIC](/PolycrystalVoronoiCoupledVoidIC.md) for each order parameter `op_num`.

## Inputs

The following are best provided as global inputs in the input file, as they must be the same for each initial condition created by this action.

- `op_num`: Number of grain order parameters used to create polycyrstal voronoi. For each "op_num" variable, a "PolycrystalVoronoiCoupledVoidIC" initial condition is automatically created.
- `invalue`: Value of the coupled void variable "v" inside the void.
- `outvalue`: Value of the coupled void variable "v" outsize the void.
- `polycrystal_ic_uo`: "UserObject" like PolycrystalVoronoi or PolycrystalHex for obtaining polycrystal grain structure.

The following can be provided locally within the action block to avoid name conflict with other blocks.

- `v`: Coupled variable associated with the void initial condition.

## Example Input File Syntax

!listing modules/phase_field/test/tests/initial_conditions/PolycrystalVoronoiCoupledVoidIC.i

!syntax description /ICs/PolycrystalICs/PolycrystalVoronoiCoupledVoidIC/PolycrystalVoronoiCoupledVoidICAction

!syntax parameters /ICs/PolycrystalICs/PolycrystalVoronoiCoupledVoidIC/PolycrystalVoronoiCoupledVoidICAction
