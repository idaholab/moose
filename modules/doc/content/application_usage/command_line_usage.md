# Command-Line Usage

## Command-Line Options

All MOOSE-based applications come with quite a few command-line options.  These can be customized by each application by adding "Command Line Parameters" to their `MooseApp` derived object.

Command-line options can be set using either short syntax such as `-i inputfile.i` or longer syntax with either `--long-option value` or `--long-option=value`.  If spaces are needed for the value then you need to quote them like `--long-option='value1 value2'`.  If using the `=` it's important not to put any space around it.

To print out the available command-line options use `--help`.  An example from MooseTest looks like this:

```
> ./moose_test-opt --help

Usage: ./moose_test-opt [<options>]

Options:
  --check-input                                     Check the input file (i.e. requires -i <filename>) and quit.
  --color [auto,on,off]                             Whether to use color in console output (default 'on').
  --definition                                      Shows a SON style input definition dump for input validation
  --disallow-test-objects                           Don't register test objects and syntax
  -v --version                                      Print application version
  --distributed-mesh                                The libMesh Mesh underlying MooseMesh should always be a DistributedMesh
  --dump [search_string]                            Shows a dump of available input file syntax.
  --error                                           Turn all warnings into errors
  --error-deprecated                                Turn deprecated code messages into Errors
  -o --error-override                               Error when encountering overridden or parameters supplied multiple times
  -e --error-unused                                 Error when encountering unused input file options
  --half-transient                                  When true the simulation will only run half of its specified transient (ie half the timesteps).  This is useful for testing recovery and restart
  -h --help                                         Displays CLI usage statement.
  -i <input_file>                                   Specify an input file
  --json                                            Dumps input file syntax in JSON format.
  --keep-cout                                       Keep standard output from all processors when running in parallel
  --list-constructed-objects                        List all moose object type names constructed by the master app factory.
  --mesh-only [mesh_file_name]                      Setup and Output the input mesh only (Default: "<input_file_name>_in.e")
  --minimal                                         Ignore input file and build a minimal application with Transient executioner.
  --n-threads=<n>                                   Runs the specified number of threads per process
  --no-color                                        Disable coloring of all Console outputs.
  --no-timing                                       Disabled performance logging. Overrides -t or --timing if passed in conjunction with this flag
  --no-trap-fpe                                     Disable Floating Point Exception handling in critical sections of code when using DEBUG mode.
  --recover [file_base]                             Continue the calculation.  If file_base is omitted then the most recent recovery file will be utilized
  --recoversuffix [suffix]                          Use a different file extension, other than cpr, for a recovery file
  --redirect-stdout                                 Keep standard output from all processors when running in parallel
  -r <n>                                            Specify additional initial uniform refinements for automatic scaling
  --registry                                        Lists all known objects and actions.
  --registry-hit                                    Lists all known objects and actions in hit format.
  --show-controls                                   Shows the Control logic available and executed.
  --show-input                                      Shows the parsed input file before running the simulation.
  --show-outputs                                    Shows the output execution time information.
  --split-file [filename]                           optional name of split mesh file(s) to write/read
  --split-mesh [splits]                             comma-separated list of numbers of chunks to split the mesh into
  --syntax                                          Dumps the associated Action syntax paths ONLY
  -t --timing                                       Enable all performance logging for timing purposes. This will disable all screen output of performance logs for all Console objects.
  --trap-fpe                                        Enable Floating Point Exception handling in critical sections of code.  This is enabled automatically in DEBUG mode
  --use-split                                       use split distributed mesh files
  -w --allow_unused                                 Warn about unused input file options
  --yaml                                            Dumps input file syntax in YAML format.

Solver Options:
  See solver manual for details (Petsc or Trilinos)
```

## Important Options

Below are a few important command-line options you should be aware of:

### `-i`

The most important option is `-i` this is how you specify an input file to read like so:

```
./yourapp-opt -i input.i
```

It's always important to `cd` to the directory where your input file is so that relative paths within the input file are treated properly.

### `--dump`

`--dump` will show you all of the available input file syntax for your application.  This can be quite overwhelming so `--dump` can optionally take an argument for a piece of syntax to search for like so:

```
./yourapp-opt --dump SomeKernel
```

Would show you documentation for objects matching `SomeKernel`.

### `--recover`

If you output checkpoint files (using `checkpoint = true` in your `Outputs` block in your input file) then `--recover` will allow you to continue a solve that died in the middle of the solve.  This can allow you to recover a job that was killed because the power went out or your job ran out of time on the cluster you were using.

Again: you *MUST* turn on `checkpoint = true` in the `Outputs` block of your input file for this to work!  We now recommend that all input files contain `checkpoint = true`.

### `--n-threads`

`--n-threads` controls the number of threads per MPI process MOOSE will use for the computation.  This is how you turn on shared-memory parallelism.

### Mesh Splitting Options

For more information see [the Splitting documentation under the Mesh System](/splitting.md)

## Command-Line Input File Overrides

Any input file parameters can be overriden/set from the command-line.  This is incredibly handy for scripting and parameter studies.  The way it works is that you use a "directory" type of syntax.  Let's say that you have this `[Kernels]` block in your input file:

```conf
[Kernels]
  [./akernel]
    type = MyKernel
    variable = somevar
    coefficient = 0.2
  [../]
[]
```

To set the value of `coefficient` from the command-line you would run your application like so:

```
./yourapp-opt -i theinput.i Kernels/akernel/coefficient=0.7
```

It's important to remember not to use any spaces when doing command-line overrides like this.
