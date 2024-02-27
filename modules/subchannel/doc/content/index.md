!config navigation breadcrumbs=False scrollspy=False

# MOOSE-SubChannel class=center style=font-weight:600;font-size:350%

!style halign=center
MOOSE-SubChannel is a Subchannel application for performing reactor core, single-phase thermal-hydraulic
simulations, for bare rod, square lattice bundles or wire-wrapped/bare rod, triangular lattice bundles. It can model water cooled or metal cooled (lead, sodium, LBE) bundles. It is based on the [MOOSE framework](https://mooseframework.inl.gov), and builds on the framework and modules for many of its capabilities.

!row!
!col! small=12 medium=4 large=4 icon=get_app

## [Getting Started](getting_started/installation.md) class=center style=font-weight:200;font-size:150%;

!style halign=center
Quickly learn how to obtain the MOOSE-SubChannel source code, compile an executable, and
run simulations with the instructions found [here](getting_started/installation.md).
!col-end!

!col! small=12 medium=4 large=4 icon=settings

## [Code Reference](syntax/index.md) class=center style=font-weight:200;font-size:150%;

!style halign=center
MOOSE-SubChannel provides capabilities that can be applied to a wide variety of problems.
The Code Reference provides detailed documentation of specific code features.
General user notes on MOOSE-SubChannel can also be found [here](getting_started/user_notes.md).
!col-end!

!col! small=12 medium=4 large=4 icon=assessment

## [Verification & Validation](v&v/v&v-list.md) class=center style=font-weight:200;font-size:150%

!style halign=center
Several problems originally developed for thermal-hydraulic subchannel codes have been used for the
validation and verification of Pronhorn-SC. These cases can be found [here](v&v/v&v-list.md).
!col-end!
!row-end!

## `MOOSE-SubChannel` is a `MOOSE` application style=clear:both

!style halign=left
MOOSE-SubChannel is a [MOOSE] thermal hydraulic subchannel analysis application. It leverages the [PETSc](https://petsc.org/release/) library capabilities to solve the subchannel equations using an Newton non-linear solver. It permits seamless coupling with other [MOOSE] applications/modules like [BISON](https://mooseframework.inl.gov/bison/)/[heat-conduction](https://mooseframework.inl.gov/modules/heat_conduction/index.html). This affords the solution of coupled physics problems of varying size and dimensionality. These can be solved using computer hardware appropriate for the model size, ranging from
laptops and workstations to large high performance computers.

!media large_media/framework/inl_blue.png style=float:right;width:30%;margin-left:30px;

Code reliability is a central principle in code development, and this project
employs a well-defined development and testing strategy.  Code changes are only
merged into the repository after both a manual code review and the automated
regression test system have been completed.  The testing process and status of
MOOSE-SubChannel is available at [civet.inl.gov](https://civet.inl.gov/repo/530/).

MOOSE-SubChannel and MOOSE are developed at Idaho National Laboratory by a team of
computer scientists and engineers and is supported by various funding agencies,
including the [United States Department of Energy](http://energy.gov).  Development
of these codes is ongoing at [INL](https://www.inl.gov) and by collaborators
throughout the world.
