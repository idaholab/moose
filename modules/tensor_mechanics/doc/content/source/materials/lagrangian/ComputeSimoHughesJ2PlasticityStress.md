# ComputeSimoHughesJ2PlasticityStress

!syntax description /Materials/ComputeSimoHughesJ2PlasticityStress

## Overview

This class provides a hyperelastic Neo-Hookean stress update with J2 plasticity.

## Example Input File Syntax

The follow example configures a large deformation Neo-Hookean model with J2 plasticity and linear hardening.

!listing modules/tensor_mechanics/test/tests/lagrangian/materials/correctness/hypereltic_J2_plastic.i
         block=Materials

!syntax parameters /Materials/ComputeSimoHughesJ2PlasticityStress

!syntax inputs /Materials/ComputeSimoHughesJ2PlasticityStress

!syntax children /Materials/ComputeSimoHughesJ2PlasticityStress
