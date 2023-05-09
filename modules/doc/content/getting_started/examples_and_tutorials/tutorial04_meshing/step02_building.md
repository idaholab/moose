# Building the Reactor Module

To use the Reactor Module:

1. [Install MOOSE](getting_started/installation/index.md)

   !alert note
   In the following instructions, `$MOOSE_DIR` is defined as the MOOSE installation directory. This can be set in a Bash shell like this (assuming you installed in `~/projects/moose`):

   ```bash
   export $MOOSE_DIR=~/projects/moose
   ```

2. Compile the Reactor Module App by navigating to `$MOOSE_DIR/modules/reactor` and issuing the `make` command. This will generate a binary file in that directory which leverages both the
   MOOSE framework and Reactor module capabilities. Running the binary with the `--version` flag should simply output the current code version without error if it was compiled properly.

   ```bash
   cd $MOOSE_DIR/modules/reactor
   make -j4
   ./reactor-opt --version
   ```

3. Alternatively, compile the Combined App which produces a binary that includes all of MOOSE's modules:

   ```bash
   cd $MOOSE_DIR/modules/combined
   make -j4
   ./combined-opt --version
   ```

4. Alternatively, the user may enable the Reactor Module in a custom physics application by modifying the application's `Makefile` according to [getting_started/new_users.md].

Note that the Griffin application is auto-configured to leverage the Reactor Module and capabilities are therefore automatically available in Griffin.

To test that a compiled binary is linked properly with the Reactor module, the binary can be run with one of the Reactor module test inputs:

```bash
$MOOSE_DIR/modules/reactor/reactor-opt -i $MOOSE_DIR/modules/reactor/test/tests/meshgenerators/simple_hexagon_generator/sim_hex.i --mesh-only
```

This should output generated mesh information without producing any errors.

!content pagination previous=tutorial04_meshing/step01_overview.md
                    next=tutorial04_meshing/step03_workflow.md
