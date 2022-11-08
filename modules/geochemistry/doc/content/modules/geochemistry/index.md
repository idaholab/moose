# The geochemistry module

## Overview

The geochemistry module is designed to solve geochemical models.  The capabilities include:

- equilibrium aqueous systems
- redox disequilibrium
- sorption and surface complexation
- kinetics
- all these combined with fluid and heat transport

The geochemistry module is designed to interface easily with the porous-flow module so that complicated reactive transport scenarios can be studied.

!media geotes_weber_tensleep_3D.png caption=Temperature, porosity, pH and free volume of Quartz after 90 days of injection in a 3D reactive-transport model.  id=geotes_wt_3Da.fig

## Details

- [Theory, numerical solution technique and computational aspects](geochemistry/theory/index.md)
- [Examples](geochemistry/tests_and_examples/index.md)
- [Simulating biogeochemistry](theory/biogeochemistry.md)
- [Description of input-file objects](geochemistry/systems.md)
- [A to Z index of these webpages](geochemistry/contents.md)

## Installation and usage

After installing MOOSE using the "Getting Started" instructions (above), only the "framework" will have been compiled.  To compile any of the physics modules, including the geochemistry module, use the following instructions run from the command line:

```bash
cd ~/projects/moose/modules
make
cd ~/projects/moose/modules/geochemistry
make
cd ~/projects/moose/modules/geochemistry/unit
make
```

(If your computer has $N$ cores, the `make` process may be sped up by using the command `make -j`$N$ instead of simply `make`.)

Check that the geochemistry module is correctly compiled using the following instructions:

```bash
cd ~/projects/moose/modules/geochemistry/unit
./run_tests
cd ~/projects/moose/modules/geochemistry
./run_tests
```

Virtually all the tests should run and pass.  Some may be "skipped" due to a particular computer setup (for instance, not enough threads).  (If your computer has $N$ cores, the `run_tests` command may be sped up by using the command `./run_tests -j`$N$ instead of simply `./run_tests`.)

The geochemistry executable is called `geochemistry-opt` and is found at `~/projects/moose/modules/geochemistry`.  This may be used to run pure geochemistry simulations.  For example, to run the [cooling a solution in contact with feldspars example](tests_and_examples/cooling_feldspar.md):

```bash
cd ~/projects/moose/modules/geochemistry/test/tests/time_dependent_reactions
../../../geochemistry-opt -i cooling.i
```

For coupled reactive-transport simulations using the PorousFlow module, the `combined-opt` executable must be used.  For example, to run the [Weber-Tensleep GeoTES example](tests_and_examples/geotes_weber_tensleep.md) from the command line:

```bash
cd ~/projects/moose/modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep
../../../combined-opt -i exchanger.i
```
