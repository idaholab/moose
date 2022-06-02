# SLKKSSum

!syntax description /Kernels/SLKKSSum

## Overview

This kernel is used if the simulation only contains a single phase with multiple
sublattices. Otherwise
[`SLKKSMultiPhaseConcentration`](SLKKSMultiPhaseConcentration.md) needs to be used.

One potential application is a static solve for the sublattice concentrations
(and free energy) of a single phase. This information can be used to tabulate
sublattice concentrations as functions of total concentrations to inform
initial conditions of full solve simulations.

### See also

- `modules/phase_field/examples/slkks/CrFe_sigma`  for an example input that tabulates the sublattice concentrations of the Fe-Cr sigma phase
- `modules/phase_field/examples/slkks/CrFe`  for an example input for a full solve that uses the tabulated sublattice concentrations to set the initial conditions

## Example Input File Syntax

!! Describe and include an example of how to use the SLKKSSum object.

!syntax parameters /Kernels/SLKKSSum

!syntax inputs /Kernels/SLKKSSum

!syntax children /Kernels/SLKKSSum
