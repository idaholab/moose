# Introduction

!---

## MOOSE

- Started in May of 2008
- A framework enabling rapid development of new simulation tools
- NQA-1 Compliant
- Application development focuses on implementing physics (PDEs) rather than numerical implementation issues
- Seamlessly couples native (MOOSE) applications using MOOSE MultiApps and Transfers
- Efficiently couples non-native codes using MOOSE-Wrapped Apps
- Open Sourced February 12, 2014

!media images/moose.png
       style=width:40%;margin-left:auto;margin-right:auto;display:block;background:white;

!---

## Coupling

!row!
!col! width=50%
- Loosely-Coupled

  - Each physics solved with a separate linear/nonlinear solve.
  - Data exchange once per timestep (typically)

- Tightly-Coupled / Picard

  - Each physics solved with a separate linear/nonlinear solve.
  - Data is exchanged and physics re-solved until “convergence”

- Fully-Coupled

  - All physics solved for in one linear/nonlinear solve

!col-end!

!col width=50%
!media images/coupling.png
       style=width:80%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!row-end!


!---

## MultiApps and Transfers

- MOOSE was originally created to solve fully-coupled systems of PDEs.
- Not all systems need to be / are fully coupled:

  - Multiscale
  - Systems with multiple timescales.
  - Coupling to external codes.

- To MOOSE these situations look like loosely-coupled systems of fully-coupled equations.
- The MultiApp system allows multiple MOOSE (or external) applications to run simultaneously in parallel.
- A single MultiApp might represent thousands of individual solves.
- The Transfer system in MOOSE is designed to push and pull fields and data to and from MultiApps.

!---

## MultiApps

!row!
!col! width=50%
- MOOSE-based solves can be nested to achieve multiscale Multiphysics simulations

  - Macroscale simulations can be coupled to embedded microstructure simulations

- Arbitrary levels of solves
- Each solve is spread out in parallel to make the most efficient use of computing resources
- Efficiently ties together multiple team’s codes
- MOOSE-wrapped, external applications can exist anywhere in this hierarchy
- Apps do NOT know they are running within the MultiApp hierarchy!
!col-end!

!col width=50%
!media images/multiapp_hierarchy.png
       style=width:100%;margin-left:auto;margin-right:auto;display:block;

!row-end!

!---

## Transfers

- Transfers allow you to move data between MultiApps
- Three main catergories of Transfers exist in MOOSE:

  - Field Mapping

    - L2 Projection, Interpolation, Evaluation

  - “Postprocessed” Spatial Data

    - e.g.: Layered Integrals and Averages, Assembly Averaged Data, etc.

  - Scalar Transfer

    - Postprocessor values (Integrals, Averages, Point Evaluations, etc.)
    - Can be transferred as a scalar or interpolated into a field.
    - Useful for multi-scale

- All Transfers are agnostic of dimension (if it makes sense!)
- When transferring to or from a MultiApp a single Transfer will actually transfer to or from ALL sub-apps in that Multi-App simultaneously.
