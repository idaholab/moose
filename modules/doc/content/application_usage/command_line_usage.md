# Command-Line Usage

## Command-Line Options

All MOOSE-based applications come with quite a few command-line options.  These can be customized by each application by adding "Command Line Parameters" to their `MooseApp` derived object.

Command-line options can be set using either short syntax such as `-i inputfile.i` or longer syntax with either `--long-option value` or `--long-option=value`.  If spaces are needed for the value then you need to quote them like `--long-option='value1 value2'`.  If using the `=` it's important not to put any space around it.

An option that is designated as "global" means that it will also be passed to any MultiApps that exist in the simulation.

To print out the available command-line options use `--help`.  An example from MooseTest looks like this:

```
> Usage: moose_test-opt [<options>]

Options:
  --app <type>                        Specify the application type to run (case-sensitive)
  --copy-inputs <dir>                 Copies installed inputs (e.g. tests, examples, etc.) to a directory <appname>_<dir>
  --definition                        Shows a SON style input definition dump for input validation
  --disallow-test-objects             Don't register test objects and syntax
  -v --version                        Print application version
  --dump                              Shows a dump of available input file syntax
  --dump-search <search>              Shows a dump of available input syntax matching a search
  -h --help                           Displays CLI usage statement
  -i <input file(s)>                  Specify input file(s); multiple files are merged
  --json                              Dumps all input file syntax in JSON format
  --json-search                       Dumps input file syntax matching a search in JSON format
  --language-server                   Starts a process to communicate with development tools using the language server protocol
  --libtorch-device                   The device type we want to run libtorch on.
  --list-constructed-objects          List all moose object type names constructed by the master app factory
  --mesh-only <optional path>         Build and output the mesh only (Default: "<input_file_name>_in.e")
  --minimal                           Ignore input file and build a minimal application with Transient executioner
  --output-inverse-eigenvalue         True to let EigenProblem output inverse eigenvalue.
  --output-wall-time-interval <sec>   The target wall time interval at which to write to output; for testing
  -r <num refinements>                Specify additional initial uniform mesh refinements
  --registry                          Lists all known objects and actions
  --registry-hit                      Lists all known objects and actions in hit format
  --run <test harness args>           Runs the inputs in the current directory copied to a user-writable location by "--copy-inputs"
  --docs                              Print url/path to the documentation website
  --show-copyable-inputs              Shows the directories able to be copied into a user-writable location
  --show-type                         Return the name of the application object
  --split-file <filename>             Name of split mesh file(s) to write/read
  --split-mesh <splits>               Comma-separated list of numbers of chunks to split the mesh into
  --start-in-debugger <debugger>      Start the application and attach a debugger; this will launch xterm windows using <debugger>
  --stop-for-debugger <seconds>       Pauses the application during startup for <seconds> to allow for connection of debuggers
  --syntax                            Dumps the associated Action syntax paths ONLY
  --test-check-legacy-params          Check for legacy parameter construction with CheckLegacyParamsAction; for testing
  --test_getRestartableDataMap_error  Call getRestartableDataMap with a bad name.
  --executor                          Use the new Executor system instead of Executioners
  --use-split                         Use split distributed mesh files
  --yaml                              Dumps all input file syntax in YAML format
  --yaml-search                       Dumps input file syntax matching a search in YAML format

Global Options:
  --allow-test-objects                Register test objects and syntax
  -w --allow-unused                   Warn about unused input file options instead of erroring
  --check-input                       Check the input file (i.e. requires -i <filename>) and quit
  --color <auto,on,off=on>            Whether to use color in console output
  --disable-perf-graph-live           Disables PerfGraph live printing
  --distributed-mesh                  Forces the use of a distributed finite element mesh
  --error                             Turn all warnings into errors
  --error-deprecated                  Turn deprecated code messages into Errors
  -o --error-override                 Error when encountering overridden or parameters supplied multiple times
  -e --error-unused                   Error when encountering unused input file options
  --keep-cout                         Keep standard output from all processors when running in parallel
  --n-threads=<n>                     Runs the specified number of threads per process
  --no-color                          Disable coloring of all Console outputs
  --no-gdb-backtrace                  Disables gdb backtraces.
  --no-timing                         Disabled performance logging; overrides -t or --timing
  --no-trap-fpe                       Disable floating point exception handling in critical sections of code (unused due to non-debug build)
  --perf-graph-live-all               Forces printing of ALL progress messages
  --recover <optional file base>      Continue the calculation. Without <file base>, the most recent recovery file will be used
  --redirect-stdout                   Keep standard output from all processors when running in parallel
  --show-controls                     Shows the Control logic available and executed
  --show-input                        Shows the parsed input file before running the simulation
  --show-outputs                      Shows the output execution time information
  --test-checkpoint-half-transient    Run half of a transient with checkpoints enabled; used by the TestHarness
  -t --timing                         Enable all performance logging for timing; disables screen output of performance logs for all Console objects
  --timpi-sync <type=nbx>             Changes the sync type used in spare parallel communitations within TIMPI
  --trap-fpe                          Enable floating point exception handling in critical sections of code

Solver Options:
  See PETSc manual for details
```

## Important Options

Below are a few important command-line options you should be aware of:

### `-i`

The most important option is `-i` this is how you specify an input file to read like so:

```
./yourapp-opt -i input.i
```

It's always important to `cd` to the directory where your input file is so that relative paths within the input file are treated properly.

It's also possible to specify multiple input files like the following:

```
./yourapp-opt -i base.i input.i
```

The input files are processed from left to right, so in this example, `base.i`
is processed before `input.i`. This feature is useful when you have multiple
input files that share input: you can factor out the common input into another
file (e.g., `base.i`) so that you do not duplicate it (and thus risk changing
it in one input file but not another).

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

Any input file parameters can be overridden/set from the command-line.  This is incredibly handy for scripting and parameter studies.  The way it works is that you use a "directory" type of syntax.  Let's say that you have this `[Kernels]` block in your input file:

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
