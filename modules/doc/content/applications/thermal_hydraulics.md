# Thermal Hydraulics Applications

MOOSE provides versatile, general-purpose thermal-hydraulics applications. These applications solve for mass, momentum, and energy conservation in multicomponent, multiphase flows using incompressible, weakly-compressible, or fully compressible formulations for steady-state or transients in lumped parameters and/or multidimensional (1, 2, or full 3D) geometries.

These applications are developed as part of the
[Nuclear Energy Advanced Modeling and Simulation (NEAMS) program](https://neams.inl.gov/).

!media thermal_hydraulics/misc/TH_scales_new.png
       style=width:50%;display:block;margin-left:auto;margin-right:auto;
       caption=MOOSE modules support from Reynold-Average Navier Stokes (RANS) Computational Fluid Dynamics (CFD) modeling to 0D lumped parameters modeling.
       id=TH_Scales

## MOOSE-based Modules for Thermal-Hydraulics Modeling

| Module                                                                  | Scale                                  | Flow-Formulation                                                                                                                                             | Dimension                                                          | Typical Element Count | Typical Runtime | Typical Simulations                                                                                                                                                                                    |
| :---------------------------------------------------------------------- | :------------------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------- | :----------------------------------------------------------------- | :-------------------- | :-------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [Navier-Stokes Modue](navier_stokes/index.md)                           | Coarse-Mesh CFD \\ \\ RANS simulations | Incompressible, Weakly-Compressible, or Fully-Compressible\\ \\ Single- or Multi-Phase \\ \\ Single- or Multi-Component Flow                                 | Typically, 2D, 2D axisymmetric, or 3D \\ \\ Can also be used in 1D | 10,000                | 1 minute        | Flow through nuclear reactor core or plena \\ \\ 3D multi-phase flow in pipes \\ \\ Natural convection flow in open cavities                                                                           |
| [Subchannel Module](https://subchannel-dev.hpc.inl.gov/site/index.html) | Subchannel Scale                       | Incompressible or Weakly-Compressible \\ \\ Single-Phase \\ \\ Single- or Multi-Component Flow                                                               | Typically, 3D \\ \\ Can be used in 1D and 2D                       | 100,000               | 10 seconds      | Flow development through nuclear reactor fuel assembly \\ \\ Thermal-hydraulics analysis of nuclear reactor assembly blockage \\ \\ Natural convection cooling in nuclear reactors low-flow assemblies |
| [Thermal-Hydraulics Module](modules/thermal_hydraulics/index.md)        | Lumped-Parameters Simulations          | Compressible \\ \\ Single-Phase; Single-Component Flow                                                                                                       | 1D, 0D                                                             | 100                   | 10 seconds      | Heat extraction unit from nuclear reactor core \\ \\ Thermal loops with significant compressibility effects                                                                                            |
| [Porous Flow Module](modules/porous_flow/index.md)                      | Coarse-Mesh CFD                        | Incompressible, Weakly-Compressible, or Fully-Compressible Porous Flow (no inertial term) \\ \\ Single- or Multi-Phase \\ \\ Single- or Multi-Component Flow | Typically, 2D, 2D axisymmetric, or 3D \\ \\ Can also be used in 1D | 10,000                | 1 minute        | Flow through fractured porous media \\ \\ Underground coal mining \\ \\ CO storage in saline aquifers                                                                                                  |
| [Scalar Kernels System](syntax/ScalarKernels/index.md)                  | 0D ODE                                 | Incompressible                                                                                                                                               | 0D                                                                 | 10                    | 1 second        | Bernoulli-like formulations for pressure drop                                                                                                                                                          |

## NCRC Applications for Advanced Engineering Modeling

Via the [Nuclear Computational Resource Center (NCRC)](https://inl.gov/ncrc/), several additional applications are supported for enhanced engineering modeling. See [help/inl/applications.md] for more information.

| Application Name                                               | Based On                  | Added Features                                                                                                                                         |
| :------------------------------------------------------------- | :------------------------ | :----------------------------------------------------------------------------------------------------------------------------------------------------- |
| [Pronghorn](https://pronghorn-dev.hpc.inl.gov/site/index.html) | Navier-Stokes Module      | Export controlled correlations for pressure drop, heat exchange, and mass transfer in advanced nuclear reactors.                                       |
| SAM                                                            | MOOSE Framework           | Additional lumped parameters components for realistic plant modeling and proprietary correlations for pressure drop, heat exchange, and mass transfer. |
| [Sockeye](https://sockeye-dev.hpc.inl.gov/site/)               | Thermal-Hydraulics Module | Adapted correlations and specific 1D and 2D models for high-temperature heat pipes.                                                                    |

## Applications Examples Gallery

!row!
!col! small=4 medium=4 large=4

### Molten Salt Reactors

!media thermal_hydraulics/misc/example_msr.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=RANS simulation of conjugated heat transfer in a pool-type molten salt reactor concept using the MOOSE Navier-Stokes module.
       id=mcre

!media thermal_hydraulics/misc/example_msre.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Power density (left), fuel temperature (center), and void fraction distribution (right) during the steady-state operation of the Molten Salt Reactor Experiment using the MOOSE Navier-Stokes module RANS simulation.
       id=msre

!col-end!

!col! small=4 medium=4 large=4

### High Temperature Reactors

!media thermal_hydraulics/misc/example_httf.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Steady-state operation of Oregon State University's High Temperature Test Facility (HTTF) using the MOOSE Navier-Stokes module coarse-mesh CFD capability.
       id=httf

!media thermal_hydraulics/misc/example_httr.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Temperature in fuel blocks of the High-Temperature Engineering Test Reactor (HTTR) during steady-state operation using the MOOSE Navier-Stokes module coarse-mesh CFD capability.
       id=httr

!col-end!

!col! small=4 medium=4 large=4

### Liquid-Metal cooled Reactors

!media thermal_hydraulics/misc/example_subchannel.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Steady-state operation of a fuel assembly in a liquid-metal-cooled reactor using the Subchannel Module.
       id=subchannel

!media thermal_hydraulics/misc/example_subchannel_lf.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Simulation of internal recirculation in low-flow assemblies of a sodium-cooled fast reactor driven by natural convection, conducted using the Subchannel Module.
       id=subcnahhel_lf

!col-end!

!col! small=4 medium=4 large=4

### Two-Phase Flow

!media thermal_hydraulics/misc/example_rayleigh_benard.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Two-phase Rayleigh-Benard convection in a 3D cavity using the drift-flux mixture model in the Navier-Stokes module, where the flow boils at the bottom plate and condenses at the top plate.
       id=rayleigh_benard

!media thermal_hydraulics/misc/example_two_phase_channel.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Two-phase flow stratification in a flow bed using the MOOSE Navier-Stokes module Euler-Euler capabilities, illustrating phase-fraction (top), phase-specific velocities (center), and pressure (bottom).
       id=two_phase_channel

!col-end!

!col! small=4 medium=4 large=4

### Laser Welding

!media thermal_hydraulics/misc/example_laser_weld.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Melt-pool evaluation during laser welding, simulated using the MOOSE Navier-Stokes module.
       id=laser_weld

!col-end!

!col! small=4 medium=4 large=4

### Corrosion and Erosion

!media thermal_hydraulics/misc/example_corrosion.png
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Prediction of critical spots for corrosion and erosion in a double-elbow pipe using the MOOSE Navier-Stokes module.
       id=corrosion

!col-end!

!row-end!

