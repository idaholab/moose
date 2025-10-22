# MOOSE Introduction

!style halign=center
Multi-physics Object Oriented Simulation Environment

!---

!media darcy_thermo_mech/moose_intro.png
       style=width:75%;margin-left:auto;margin-right:auto;display:block;
       alt=Promotional image showing some of the different types of physics MOOSE can capture when simulating a nuclear reactor core.

!---

## History and Purpose

- Development started in 2008

- Open-sourced in 2014

- Designed to solve computational engineering problems and
  reduce the expense and time required to develop new =applications= by:

  - being easily extended and maintained
  - working efficiently on a few and many processors
  - providing an object-oriented, extensible system for creating all aspects of a simulation tool

!---

## MOOSE By The Numbers

- 250 contributors
- 56,000 commits
- 5000 unique visitors per month
- ~40 new Discussion participants per week
- 1500 citations for the MOOSE papers

  - Most cited paper in Elsevier Software-X
  - More than 500 publications using MOOSE

- 30M tests per week

!---

!media darcy_thermo_mech/moose_design.png
       style=width:75%;margin-left:auto;margin-right:auto;display:block;
       alt=The MOOSE technology stack.

!---

## General Capabilities

- Continuous and Discontinuous Galerkin FEM
- Finite Volume
- Supports fully coupled or segregated systems, fully implicit and explicit time integration
- Automatic differentiation (AD)
- Unstructured mesh with FEM shapes
- Higher order geometry
- Mesh adaptivity (refinement and coarsening)
- Massively parallel (MPI and threads)
- User code agnostic of dimension, parallelism, shape functions, etc.
- Operating Systems:

  - macOS
  - Linux
  - Windows (WSL)

!---

## Object-oriented, pluggable system

!media darcy_thermo_mech/moose_systems.png
       style=width:75%;margin-left:auto;margin-right:auto;display:block;
       alt=The components of MOOSE.

!---

## Example Code

!media darcy_thermo_mech/moose_code.png
       style=width:100%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;
       alt=The relationship between the strong form of an equation, the weak form, and the code for one of the terms.

!---

## Software Quality

- MOOSE follows an Nuclear Quality Assurance Level 1 (NQA-1) development process
- All commits undergo review using GitHub Pull Requests and must pass a set of application
  regression tests before they are available to our users
- MOOSE includes a test suite and documentation system to allow for agile development while
  maintaining a NQA-1 process
- Utilizes the Continuous Integration Environment for Verification, Enhancement, and Testing (CIVET)
- External contributions are guided through the process by the team, and are very welcome!

!---

## Development Process

!media darcy_thermo_mech/civet_flow.png
       style=width:100%;margin-left:auto;margin-right:auto;display:block;
       alt=Flowchart for developing MOOSE, including continuous integration.

!---

## Community

###### https://github.com/idaholab/moose/discussions

!media darcy_thermo_mech/moose_users.png
       style=width:50%;margin-left:auto;margin-right:auto;display:block;
       alt=Screenshot of MOOSE discussions on GitHub.

!---

!media darcy_thermo_mech/moose_contributors.png
       style=width:80%;margin-left:auto;margin-right:auto;display:block;background:white;
       alt=Plot of the total number of contributions and unique contributors to MOOSE, from 2008 to 2023.

!---

## License

- LGPL 2.1
- Does not limit what you can do with your application

  - Can license/sell your application as closed source

- Modifications to the library itself (or the modules) are open source
- New contributions are automatically LGPL 2.1
