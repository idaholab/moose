# Using SCM

## Running an Input File and Viewing Results

!style halign=left
After SCM is installed and tested, you should now be able to run input files
using the `subchannel-opt` executable located at `~/projects/subchannel`. Input files
demonstrating the capabilities of SCM can be found in `~/projects/subchannel/examples`.
Any input file (say, one called `example_input.i`) can be run with the following
basic syntax:

```bash
~/projects/SubChannel/subchannel-opt -i example_input.i
```

Information about the simulation and its progress will then be displayed on the
screen. If a log file of the iterative solution process is also desired, the
console output can be sent to a text file:

```bash
~/projects/SubChannel/subchannel-opt -i example_input.i --color off 2>&1 | tee log.txt
```

!alert note
The `--color off` option is to de-clutter the log file from extraneous
mark-up resulting from the displayed console text colors.

Many test input files also create an output file with the name format
`example_input_out.e` that contains all the simulation results that have been
selected for output. These results are best viewed using a visualization tool
like [Paraview](http://www.paraview.org/download/).

## SCM Examples and Tests

!style halign=left
In general the validation examples located within the `test/tests` directory or the `examples` directory and are meant to be a showcase of "production" SCM capability. Users are encouraged to start there
when learning SCM and setting up new simulations. Documentation for the validation
examples can be found [on the SCM validation page](modules/subchannel/v&v/v&v-list.md). The
`test/tests` directory also holds *all* SCM regression tests for both complete and in-progress capability. These files *should not* be modified, so that periodic tests of SCM function can be performed successfully. If modification of example inputs is desired, they should be copied and run elsewhere.

As SCM is further developed and more capabilities are added, the tests directory will continue to
grow. If you feel that you have added important new functionality, please create a test for
it, such that any future changes will not break that capability.
