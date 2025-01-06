# SCM: SubChannel Module class=center style=font-weight:600;font-size:350%

!style halign=center
SCM is a Subchannel Module for performing reactor core, single-phase thermal-hydraulic
subchannel simulations, for bare rod, square lattice bundles or wire-wrapped/bare rod, triangular lattice bundles. It can model water cooled or metal cooled (lead, sodium, LBE) bundles. It is based on the [MOOSE framework](https://mooseframework.inl.gov), and builds on the framework and modules for many of its capabilities.

!row!

!col! small=12 medium=4 large=4 icon=school

## [General Information](general/subchannel_theory.md) class=center style=font-weight:200;

!style halign=center
Familiarize yourself with the theory behind SCM, some important user notes and how to run the SCM input file. Browse through the following information:

- [Theory](general/subchannel_theory.md)
- [Running SCM input file](general/using_SubChannel.md)
- [User Notes](general/user_notes.md)
- [Publication List](general/publication_list.md)
!col-end!

!col! small=12 medium=4 large=4 icon=settings

## [SCM Kernels](modules/subchannel/syntax.md) class=center style=font-weight:200;font-size:150%

!style halign=center
SCM specific Kernel syntax.
!col-end!

!col! small=12 medium=4 large=4 icon=assessment

## [Verification & Validation](modules/subchannel/v&v/v&v-list.md) class=center style=font-weight:200;font-size:150%

!style halign=center
Several problems that are ideal for thermal-hydraulic subchannel analysis have been used for the validation and verification of SCM. These problems can be found [here](modules/subchannel/v&v/v&v-list.md).
!col-end!
!row-end!

## `MOOSE` style=clear:both

!style halign=left
SCM is a [MOOSE] thermal hydraulic subchannel analysis module. It leverages the [PETSc](https://petsc.org/release/) library capabilities to solve the subchannel equations using an Newton non-linear solver. It permits seamless coupling with other [MOOSE] applications/modules like [BISON](https://mooseframework.inl.gov/bison/)/[heat-conduction](https://mooseframework.inl.gov/modules/heat_conduction/index.html). This affords the solution of coupled physics problems of varying size and dimensionality. These can be solved using computer hardware appropriate for the model size, ranging from
laptops and workstations to large high performance computers.

!media large_media/framework/inl_blue.png style=float:right;width:30%;margin-left:30px;

Code reliability is a central principle in code development, and this project
employs a well-defined development and testing strategy.  Code changes are only
merged into the repository after both a manual code review and the automated
regression test system have been completed.  The testing process and status of
SCM is available at [civet.inl.gov](https://civet.inl.gov/repo/530/).

SCM and MOOSE are developed at Idaho National Laboratory by a team of
computer scientists and engineers and is supported by various funding agencies,
including the [United States Department of Energy](http://energy.gov).  Development
of these codes is ongoing at [INL](https://www.inl.gov) and by collaborators
throughout the world.

## Tutorial style=clear:both

Tutorial can be found [here](modules/subchannel/tutorial/index.md)
