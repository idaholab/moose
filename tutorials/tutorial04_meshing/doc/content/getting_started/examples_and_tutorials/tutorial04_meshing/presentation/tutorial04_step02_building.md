# Building the Reactor Module

!---

## Install MOOSE

To use the Reactor Module you must first [Install MOOSE](getting_started/installation/index.md).

In this tutorial, `$MOOSE_DIR` is defined as the MOOSE installation directory. This can be set in a Bash shell like this (assuming you installed in `~/projects/moose`):

```bash
export $MOOSE_DIR=~/projects/moose
```

The copy/paste commands in this tutorial expect you will have this environment variable defined.

!---

## Compiling an app

Compile the Reactor Module App by navigating to `$MOOSE_DIR/modules/reactor` and issuing the `make` command. This will generate a binary file (`reactor-opt`) in that directory which leverages both the MOOSE framework and Reactor module capabilities. Running the binary with the `--version` flag should simply output the current code version without error if it was compiled properly.

```bash
cd $MOOSE_DIR/modules/reactor
make -j4
./reactor-opt --version
```

To test that a compiled binary is linked properly with the Reactor module, the binary can be run with one of the Reactor module test inputs:

```bash
$MOOSE_DIR/modules/reactor/reactor-opt -i $MOOSE_DIR/modules/reactor/test/tests/meshgenerators/simple_hexagon_generator/sim_hex.i --mesh-only
```

This should output generated mesh information without producing any errors.
