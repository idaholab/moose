# MOOSE

### Multi-physics Object Oriented Simulation Environment

#### [mooseframework.inl.gov](https://mooseframework.inl.gov)

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

  - Being easily extended and maintained
  - Working efficiently on a few and many processors
  - Providing an object-oriented, extensible system for creating all aspects of a simulation tool

!---

## By The Numbers

- 250 contributors
- 58,000 commits
- 5000 unique visitors per month
- ~40 new Discussion participants per week
- 150M tests per week

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
- User code agnostic of dimension, parallelism, shape functions, etc
- Native support for executing multiphysics simulations across applications
- GPU support for execution via MFEM and Kokkos
- Operating Systems:

  - macOS (Conda, Docker)
  - Linux (Apptainer, Conda, Docker)
  - Windows (Docker, WSL)

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

- Follows an Nuclear Quality Assurance Level 1 (NQA-1) development process
- All changes undergo independent review using and must pass regression tests before merge
- Includes a test suite and documentation system to allow for agile development while maintaining a NQA-1 process
- **External contributions are guided through the process by the team, and are very welcome!**

!---

## Development Process

!media darcy_thermo_mech/civet_flow.png
       style=width:100%;margin-left:auto;margin-right:auto;display:block;
       alt=Flowchart for developing MOOSE, including continuous integration.

!---

## Community

###### [github.com/idaholab/moose/discussions](https://github.com/idaholab/moose/discussions)

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
