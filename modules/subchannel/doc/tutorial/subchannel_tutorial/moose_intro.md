# Introduction to MOOSE

!---

# MOOSE Framework: Overview

!row!
!col! width=50%

!media large_media/tutorials/darcy_thermo_mech/moose_intro.png

!col-end!

!col! width=50%

- Developed by Idaho National Laboratory since 2008
- Used for studying and analyzing nuclear reactor problems
- Free and open source (LGPLv2 license)
- Large user community
- Highly parallel and HPC capable
- Developed and supported by full time INL staff - long-term support
- https://www.mooseframework.inl.gov

!col-end!
!row-end!

!---

# MOOSE Framework: Key features

!row!
!col! width=50%

!media large_media/tutorials/darcy_thermo_mech/moose_intro.png

!col-end!

!col! width=50%

- Massively parallel computation - successfully run on >100,000 processor cores
- Multiphysics solve capability - fully coupled and implicit solver
- Multiscale solve capability - multiple application can perform computation for a problem simultaneously
- Provides high level interface to implement customized physics, geometries, boundary conditions, and material models
- Initially developed to support nuclear R&D but now widely used for non-nuclear R&D also

!col-end!
!row-end!

!---

# MOOSE Framework: Applications

!media large_media/tutorials/darcy_thermo_mech/moose_herd_2019.png style=width:100%;margin-left:auto;margin-right:auto;display:block;

!---

# MOOSE Framework: Solving specific physics

!row!
!col! width=70%

!media large_media/tutorials/darcy_thermo_mech/moose_code.png

!col-end!

!col! width=30%

- Custom "kernels" representing specific physics
- They can be developed easily and incorporated into the simulation

!col-end!
!row-end!