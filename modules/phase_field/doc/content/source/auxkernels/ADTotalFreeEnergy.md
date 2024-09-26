# ADTotalFreeEnergy

AD version of [TotalFreeEnergy.md] that uses AD material properties

## Overview

Total free energy (both the bulk and gradient parts), where the bulk free energy has been defined in a material.

## Example Input File Syntax

A bicrystal input file is available:

!listing modules/phase_field/test/tests/spherical_gaussian_bicrystal/epsilon_model_bicrystal.i

A tricrystal input file is available:

!listing modules/phase_field/examples/spherical_gaussian_tricrystal/epsilon_model_tricrystal.i


!syntax parameters /AuxKernels/ADTotalFreeEnergy

!syntax inputs /AuxKernels/ADTotalFreeEnergy

!syntax children /AuxKernels/ADTotalFreeEnergy
