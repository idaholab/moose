# Command-Line Arguments

## Useful Command-Line Arguments

The following table lists useful command line options:

| Command-line option | Description |
| :- | :- |
| `--color [auto,on,off]` | Whether to use color in console output (default 'on'). |
| `--check-input` | Check the input file (i.e. requires `-i <filename>`) and quit. |
| `--error` | Turn all warnings into errors |
| `--error-deprecated` | Turn deprecated code messages into errors |
| `-h --help` | Displays CLI usage statement. |
| `-i <input_file>` | Specify one or multiple input files. Multiple files get merged into a single simulation input. |
| `--no-color` | Disable coloring of all Console outputs. |
| `--version` | Print version of the code |
| `--recover [file_base]` | Continue the calculation. If file_base is omitted then the most recent recovery file will be utilized |
| `--recoversuffix [suffix]` | Use a different file extension, other than cpr, for a recovery file |
| `-w --allow-unused` | Warn about unused input file options instead of erroring. |
| `--disable-perf-graph-live` |  Disables PerfGraph Live Printing. |

## Command-Line Arguments for Parallel Execution

These options are related to running in parallel:

| Command-line option | Description |
| :- | :- |
| `--keep-cout` | Keep standard output from all processors when running in parallel |
| `--n-threads=<n>` | Runs the specified number of threads per process |
| `--distributed-mesh` | The libMesh Mesh underlying MooseMesh should always be a DistributedMesh |
| `--redirect-stdout` | Keep standard output from all processors when running in parallel |


## Passing Input File Parameters from Command-Line

It is possible to pass input file parameters via a command-line interface.
To do that, use the following syntax:

```
./thm-opt -i input_file.i param1=value1 param2=value2
```

This capability supports scenarios when you need to run the same input file with but slightly different parameters.

The following example explains how to use this functionality:

!style! class=snippet
Let's say we have an input file like this:

```
[Block]
  [name]
    param = 0.2
  []
[]
```

To run the above input file such that the `param` would have value of `0.1`, we do:

```
./thm-opt -i input_file.i Block/name/param=0.1
```

!style-end!

Using this functionality, we can also supply new parameters that are not included in the input file.

!alert note
+Note:+ Command line shell usually splits parameters based on spaces, so make sure you +don't+ have
spaces around `=` or anywhere else which could confuse the shell and produce an unexpected result.

## More information

To print out all available command-line options, run the executable with `--help`.
