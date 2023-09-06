# Tricrystal2CircleGrainsICAction

!syntax description /ICs/PolycrystalICs/Tricrystal2CircleGrainsIC/Tricrystal2CircleGrainsICAction

## Overview

This action creates the initial conditions (ICs) for the three order parameters used to represent a tricrystal. It creates a grain structure with the second and third grains represented by circles (2D) or spheres (3D) embedded in the first grain.

The action creates the three ICs with the [Tricrystal2CircleGrainsIC](/Tricrystal2CircleGrainsIC.md).

## Example Input File Syntax

The `Tricrystal2CircleGrainsICAction` is accessed through the `ICs/PolycrystalICs` block, as shown below.

!listing modules/phase_field/test/tests/GBAnisotropy/test1.i block=GlobalParams

!listing modules/phase_field/test/tests/GBAnisotropy/test1.i block=ICs

!syntax parameters /ICs/PolycrystalICs/Tricrystal2CircleGrainsIC/Tricrystal2CircleGrainsICAction

!syntax children /ICs/PolycrystalICs/Tricrystal2CircleGrainsIC/Tricrystal2CircleGrainsICAction
