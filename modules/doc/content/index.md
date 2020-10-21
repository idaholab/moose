!config navigation breadcrumbs=False scrollspy=False long-name=Parallel Multiphysics Finite Element Framework

# HOME style=visibility:hidden;

!media media/moose_logo.png style=display:block;margin-left:auto;margin-right:auto;width:60%;

!style halign=center fontsize=120% color=0.5 0.5 0.5
Multiphysics Object-Oriented Simulation Environment

# An open-source, parallel finite element framework class=center style=font-weight:200;font-size:200%

!row!
!col! small=12 medium=4 large=4 icon=flash_on
### Rapid Development class=center style=font-weight:200;

MOOSE provides a plug-in infrastructure that simplifies definitions of physics, material properties,
and postprocessing.
!col-end!

!col! small=12 medium=4 large=4 icon=group
### User-Focused class=center style=font-weight:200;

MOOSE includes a growing set of [physics](modules/index.md) and supports multi-scale models, enabling
collaboration across applications, time-scales, and spatial domains.
!col-end!

!col! small=12 medium=4 large=4 icon=settings
### Getting Started class=center style=font-weight:200;

MOOSE works on Mac OS, Linux, and Windows, and it is easy to [get started](getting_started/index.md).
!col-end!
!row-end!

!include upcoming_training.md

# Select MOOSE Features class=center style=font-weight:200;

- Flexible Plug-In Architecture Reducing Code Development
- [Automatic Differentiation](NonlinearSystem.md)
- Continuous Finite Element
- Discontinuous Finite Element
- Mixed (CG and DG, coupled in the same simulation)
- Massively Parallel: Hybrid MPI + Threading / OpenMP
- [Scalability Proven to Over 30,000 cores](https://ieeexplore.ieee.org/abstract/document/8638143)
- Massive Problem Sizes (1B+ elements, 100B+ unknowns)
- [Multiscale Solves](MultiApps/index.md)
- Leverages [PETSc](https://www.mcs.anl.gov/petsc) solvers
- [Physics Modules:](modules/index.md)

  - [Heat Conduction](HeatConduction.md)
  - [Geochemistry](geochemistry/index.md)
  - [Navier Stokes](navier_stokes/index.md)
  - [Solid Mechanics](syntax/Modules/TensorMechanics/index.md)
  - [Contact](contact/index.md)
  - [Porous Flow](porous_flow/index.md)
  - [Phase Field](phase_field/index.md)
  - [Level Set](level_set/index.md)
  - [XFEM](xfem/index.md)

- Unstructured Mesh (Quads, Tris, Hexes, Tets, Pyramids, Wedges, etc.)
- Curvilinear Geometry
- [Mesh Adaptivity](syntax/Adaptivity/index.md)
- [In-situ Postprocessing](Postprocessors/index.md)
- [Nonlinear Material Properties](examples/ex08_materials.md)
- [Physics-Based Damping](Dampers/index.md)
- Point Sources
- Multiple Formats for Input and Output ([Exodus](Exodus.md), [VTK](VTKOutput.md), GMSH, etc.)
- Nonlinear, Coupled ODE / PDE Systems
- Arbitrary Lagrangian-Eulerian (ALE) formulation

!media media/inl_blue.png dark_src=media/inl_white.png       style=width:30%;display:block;margin-top:3em;margin-left:auto;margin-right:auto;
