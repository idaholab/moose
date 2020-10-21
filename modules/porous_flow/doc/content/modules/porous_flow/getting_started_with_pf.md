# Getting started

## Install MOOSE

The first step in using PorousFlow is to install MOOSE. For full details, see [install MOOSE](getting_started/installation/index.md optional=True).

## Build PorousFlow

An executable containing the PorousFlow library can be built using the following commands
(from the MOOSE directory)

```bash
cd modules/porous_flow
make -jn
```

where *n* is the number of cores available. This will build an executable named *porous_flow-opt*
by default, which is an optimised version of the executable. Several versions can be built, such as an
executable useful for debugging purposes, see details of the [MOOSE build system](build_system.md optional=True).

## Test PorousFlow

PorousFlow includes a large number of [QA tests](porous_flow/tests.md) which can be run using

```bash
./run_tests -jn
```

from within the  `modules/porous_flow` directory (and where *n* is the number of tests to run concurrently).

## Create your own models

If the PorousFlow application has been built successfully, all of the tests should pass, and the user can
now begin to construct their own models. A step-by-step [tutorial](porous_flow/tutorial_00.md) and a number of [examples](porous_flow/index.md) are provided to guide new users.
