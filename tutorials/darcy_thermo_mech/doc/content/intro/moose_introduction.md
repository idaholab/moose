# MOOSE Introduction

!style halign=center
Multi-physics Object Oriented Simulation Environment

!---

!media moose_intro.png style=width:100%;

!---

## History and Purpose

- Development started in 2008

- Open-sourced in 2014

- Designed to solve computational engineering problems and
  reduce the expense and time required to develop new =applications= by:

  - being easily extended and maintained
  - working efficiently on a few and many processors
  - providing an object-oriented, pluggable system for creating all aspects of a simulation tool

!---

## MOOSE Team

!media moose_team.png style=width:100%;

!---

## Code Platform

!media moose_design.png style=width:80%;margin-left:auto;margin-right:auto;display:block;


!---

## Capabilities

- User code agnostic of dimension
- Continuous and Discontinuous Galerkin
- Fully Coupled, Fully Implicit (and explicit)
- [!ac!AD]
- Unstructured mesh with FEM shapes (Quads, Tris, Hexes, Tets, Pyramids, Wedges,...)
- Higher order geometry (curvilinear, etc.)
- Mesh Adaptivity (refinement and coarsening)
- Massively Parallel (MPI+Threads)
- User code agnostic of parallelism
- User code agnostic of shape functions
- Postprocessing
- Visualization, testing, and documentation tools

!---

## Object-oriented, pluggable system

!media moose_systems.png style=width:80%;margin-left:auto;margin-right:auto;display:block;

!---

## Example Code

!media moose_code.png style=width:100%;

!---

## Software Quality

- MOOSE follows an Nuclear Quality Assurance Level 1 (NQA-1) development process
- all commits undergo review using GitHub Pull Requests and must pass a set of application
  regression tests before they are available to our users
- MOOSE includes a test suite and documentation system to allow for agile development while
  maintaining a NQA-1 process
- Utilizes the Continuous Integration Environment for Verification, Enhancement, and Testing (CIVET)

!---

## Development Process

!media civet_flow.png style=width:100%;

!---

## Community

###### moose-users@googlegroups.com

!media moose_users.png style=width:50%;margin-left:auto;margin-right:auto;display:block;
       caption=Total posts (top) and topics (bottom) from March 1, 2018 to March 27, 2019.

!---

!media moose_contributors.png style=width:100%;margin-left:auto;margin-right:auto;display:block;

!---

!media moose_add_del.png style=width:100%;margin-left:auto;margin-right:auto;display:block;
