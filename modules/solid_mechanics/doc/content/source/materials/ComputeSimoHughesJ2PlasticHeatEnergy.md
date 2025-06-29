# ComputeSimoHughesJ2PlasticHeatEnergy

!syntax description /Materials/ComputeSimoHughesJ2PlasticHeatEnergy

## Description

The `ComputeSimoHughesJ2PlasticHeatEnergy` material computes the plastic heat energy density based on the effective stress and plastic strain rate. This material is designed to work with the Simo-Hughes J2 plasticity model and provides the energy dissipated as heat during plastic deformation.

The plastic heat energy density is computed as:

!equation
q_{plastic} = \sigma_{eff} \frac{\dot{\epsilon}^p}{\Delta t}

where:
- $q_{plastic}$ is the plastic heat energy density
- $\sigma_{eff}$ is the effective stress computed as $\sqrt{\frac{3}{2}} J ||\boldsymbol{\sigma}_{dev}||$
- $J$ is the determinant of the deformation gradient
- $\boldsymbol{\sigma}_{dev}$ is the deviatoric part of the Cauchy stress tensor
- $\dot{\epsilon}^p$ is the plastic strain rate (difference in effective plastic strain divided by time step)
- $\Delta t$ is the time step

This material declares a material property named `plastic_heat` (or `base_name_plastic_heat` if a base name is specified) that can be used as a heat source in thermal calculations.

## Example Input File Syntax


!syntax parameters /Materials/ComputeSimoHughesJ2PlasticHeatEnergy

!syntax inputs /Materials/ComputeSimoHughesJ2PlasticHeatEnergy

!syntax children /Materials/ComputeSimoHughesJ2PlasticHeatEnergy
