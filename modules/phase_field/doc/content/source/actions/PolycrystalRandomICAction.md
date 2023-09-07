# PolycrystalRandomICAction

!syntax description /ICs/PolycrystalICs/PolycrystalRandomIC/PolycrystalRandomICAction

## Overview

This action creates the initial conditions (ICs) for the `op_num` order parameters used to represent a polycrystal with a random initial condition.

The action creates the IC for each of the variables with [PolycrystalRandomIC](/PolycrystalRandomIC.md).

## Example Input File Syntax

The `PolycrystalRandomICAction` is accessed through the `ICs/PolycrystalICs` block, as shown below.

!listing modules/phase_field/examples/grain_growth/grain_growth_2D_random.i block=GlobalParams

!listing modules/phase_field/examples/grain_growth/grain_growth_2D_random.i block=ICs

!syntax parameters /ICs/PolycrystalICs/PolycrystalRandomIC/PolycrystalRandomICAction

!syntax children /ICs/PolycrystalICs/PolycrystalRandomIC/PolycrystalRandomICAction
