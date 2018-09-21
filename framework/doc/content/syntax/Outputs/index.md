<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# Outputs System

- The output system is designed to be just like any other system in MOOSE: modular and expandable.
- It is possible to create multiple output objects for outputting:

  - at specific time or timestep intervals,
  - custom subsets of variables, and
  - to various file types.

- Supports commnon parameters and sub-blocks.

```
[Outputs]
  file_base = <base_file_name> 
  exodus = true
[]
```

```
[Outputs] 
  vtk = true 
  [./console]
    type = Console
    perf_log = true
    max_rows = true
  [../]
  [./exodus]
    type = Exodus
    execute_on = 'timestep_end'
  [../] 
  [./exodus_displaced]
    type = Exodus
    file_base = displaced
    use_displaced = true
    interval = 3
  [../] 
[]
```

## Outputs and execute_on

Outputs utilize the "execute_on" parameter like other systems in MOOSE, by default:

- All objects have `execute_on = 'initial timestep_end'`, with `Console` objects being the exception.
- `Console` objects append `'initial timestep_begin linear nonlinear failed'` to the "execute_on" settings at the common level.

When debugging output, use the `--show-outputs` flag when executing your application. This will add a section near the top of the simulation that shows the settings being used for each output object.

## Basic Input File Syntax

- To enable output an application it must contain an `[Outputs]` block.
- The simplest method for enabling output is to utilize the short-cut syntax as shown below, which enables `Exodus` output for writing data to a file.

```
[Outputs]
  exodus = true   # output to ExodusII file with default settings
[]
```

## Full Block Syntax

- To take full advantage of the output system the use of sub-blocks is required.
- These blocks are identical to the previous section, including the block names (i.e. the short-cut syntax builds these exact blocks).
- The resulting filenames produced by different syntax is important and discussed in the File Output Names section.

```
[Outputs] 
  [./console] 
    type = Console   # output to the screen with default settings
  [../]
  [./exodus]
    type = Exodus    # output to ExodusII file with default settings
  [../]
[]
```

- The sub-block syntax allows for increased control over the output and allows for multiple outputs of the same type to be specified.
- The following creates two `Exodus` outputs, one outputting the mesh at every time step including the initial condition, the other outputs every 3 ime steps without the initial condition. Additionally, performance logging was enable for `Console` output.

```
[Outputs] 
  execute_on = 'timestep_end' # Limit the output to timestep end (removes initial condition)
  [./console]
    type = Console
    perf_log = true          # enable performance logging
  [../]
  [./exodus]
    type = Exodus
    execute_on = 'initial timestep_end' # output the initial condition for the file
  [../]
  [./exodus_3]               # create a second output using a different interval 
    type = Exodus
    file_base = exodus_3     # set the file base (the extension is automatically applied) 
    interval = 3             # only output every third step
  [../]
[]
```

## Common Output Parameters

- The `Outputs` block also supports common parameters.
- For example, `execute_on` may be specified outside of individual sub-blocks, indicating that all-sub-blocks should output the initial condition. If within a sub-block the parameter is given a different value, the sub-block parameter takes precedence.
- The input file snippet below demonstrates the usage of common values as well as the use of multiple output blocks.
- `Console`-based outputs have special ibheritance of common parameters

  - The `execute_on` parameter, as detailed below, does not get inherited.

```
[Outputs]
  execute_on = 'timestep_end' # disable the output of initial condition
  vtk = true              # output VTK file with default setting
  print_perf_log = true   # enable the performance log
  [./exodus]
    type = Exodus
    execute_on = 'initial' # this ExodusII files will contain ONLY the initial condition
  [../]
  [./exodus_displaced]
    type = Exodus
    file_base = displaced
    use_displaced = true
    interval = 3          # this ExodusII will only output every third time step
  [../]
[]
```

## Output Execution

- In similar fashion to many other systems in MOOSE, ot is possible to control when output occurs via the "execute_on" parameter.
- By default, `execute_on = 'initial timestep_end'`
- A convenience parameter, "additional_execute_on", allows appending flags to the existing "execute_on" flags.
- The toggles shown in Output Execution Toggles operate by appending to the "execute_on" flags.
- Currently, the following output execution times are available:

| Text Flag | Description |
| :- | :- |
| "initial" | executes the output on the initial condition (on by default) |
| "linear" | executes the output on each linear iteration |
| "nonlinear" |  executes the output on each non-linear iteration |
| "timestep_end" | calls the output method at the end of the timestep (on by default) |
| "timestep_begin" | executes the output method at the beginning of the timestep |
| "final" | calls the output method on the final timestep |
| "failed" | executes the output method when the solution fails |

- As detailed in the Creating Custom Output Object section,there are two types of outputs classes: `BasicOutput` and `AdvancedOutput`.
- Advanced Outputs have additional control beyond `execute_on`, as detailed in the table below:

| Input Parameter | Method Controlled |
| :- | :- |
| execute_postprocessors_on | `outputPostprocessors` |
| execute_vector_postprocessors_on | `outputVectorPostprocessors` |
| execute_elemental_on | `outputElementalVariables` |
| execute_nodal_on | `outputNodalVariables` |
| execute_scalar_on | `outputScalarVariables` |
| execute_system_information_on | `outputSystemInformation` |
| execute_input_on | `outputInput` |

- Each of the output controls accept the same output execution flags that `execute_on` utilizes.
- In `AdvancedOutput` objects, the 1execute_on` settings are used to set the default of each of the output type controls.

  - For example, setting `execute_on = 'initial timestep_end'` causes all of the output types (e.g. postprocessors, scalars, etc.) to execute on each timestep and the initial condition.

## File Output Names

- The default naming scheme for output files utilizes the input file name (e.g. input.i) with a suffix that differs depending on how the output is defined:

  - An "_out" suffix is used for `Outputs` created using the short-cut syntax.
  - sub-blocks use the actual sub-block name as the suffix.

- For example, if the input file (input.i) contained the following [Outputs] block, two files would be created: input_out.e and input_other.e.

```
[Outputs]
  console = true
  exodus = true    # creates input_out.e
  [./other]        # creates input_other.e
     type = Exodus
     interval = 2
  [../]
[]
```

- Note, the use of `file_base` anywhere in the `[Outputs]` block disables all default naming behavior.

## Mesh Displacements

- The displaced mesh may be output by setting the 'use_displaced = true' within your output sub-blocks
- To output both the original and displaced mesh an additional output block is required.
- It is possible to perform oversampling on the dispaced mesh, simply enable both.

```
[Outputs]
  console = true
  exodus = true
  [./exodus_oversample]
     type = Exodus
     refinements = 2
     file_base = oversample
     use_displaced = true
  [../]
[]
```

## In-block Output Control

- For certain objects it is possible to control output within the block itself, these objects include: Postprocessors, Markers, Variables and AuxVariables
- For example, consider the following `[Outputs]` block that has three output objects defined.

```
[Outputs]
  exodus = true
  csv = true
  [./vtk]
    type = VTK
    interval = 2
  [../]
[]
```

- Within the sub-blocks of the types listed above it is possible to control which outputs the value will be output using the "outputs" parameter.
- For illustration purposes, consider the Postprocessors block shown below:

```
[Postprocessors]
  [./pp]
    type = EmptyPostprocessor
    outputs = 'csv'
  [../]
[]
```

- The outputs that should include the postprocessor value may be listed in the "outputs" parameter.
- The "outputs" parameter limits the defined postprocessor to output only to the `csv` output object. This highlights the need to understand the naming convention utilized by the short-cut syntax, see Advanced Syntax section.
- It is also possible to remove the postprocessor from all outputs by specifying "outputs = none".

## Non-linear/linear Residual Output

- Any object inheriting from `PetscOutput` has the ability to output data on non-linear and/or linear iterations.
- To enable this add "nonlinear" and/or "linear" to the "execute_on" input parameter.

```
[Outputs]
  [./exodus]
    type = Exodus
    execute_on = 'timestep_end linear nonlinear'
  [../]
[]
```

- When outputting nonlinear/linear iterations the time is changed from the actual simulation time by creating pseudo time steps.
- For example, if the `[Outputs]` block above was associated with a simulation that was taking a time step of 0.1 the resulting output times would use the following convention:

| Output Time | Description |
| :- | :- |
| 0.2        | Converged solve for time = 0.2 |
| 0.2001     | First non-linear iteration for time = 0.3 |
| 0.2001001  | First linear iteration for the first non-linear iteration |
| 0.2001002  | Second linear iteration for the first non-linear iteration |
| 0.2002     | Second non-linear iteration for time = 0.3 |
| 0.2002001  | First linear iteration for the second non-linear iteration |
| 0.2002002  | Second linear iteration for the second non-linear iteration |
| 0.3        | Converged solve for time = 0.3 |

## Supported Types and Syntax

| Format | Short-cut | Sub-block Type | C++ Object | Comments |
| :- | :- | :- | :- | :- |
| Console (screen) output | `console`   | `Console`   | `Console`  | Writes to the screen and optionally a file |
| Exodus II (recommend) | `exodus`    | `Exodus`     | `Exodus`    | The most common,well supported, and controllable output type |
| [VTK](http://www.vtk.org) | `vtk`       | `VTK`           | `VTKOutput` | Visual Analysis Toolkit format, requires `--enable-vtk` when building libMesh|
| [GMV](http://www.generalmeshviewer.com)            | `gmv`       | `GMV`           | `GMVOutput` | General Mesh Viewer format |
| [Nemesis](http://www.generalmeshviewer.com)  | `nemesis` | `Nemesis` | `Nemesis` | Parallel Exodus II format |
| [Tecplot](www.tecplot.com)  | `tecplot`  | `Tecplot`           | `Tecplot` | Support is limited, requires `--enable-tecplot` when building libMesh |
| [XDA](libmesh.sourceforge.net/xda_format/xda_format.pdf) | `xda` | `XDA` | `XDA` | libMesh internal format (ascii) |
| [XDR](libmesh.sourceforge.net/xda_format/xda_format.pdf)| `xdr` | `XDR` | `XDA` | libMesh internal format (binary) |
| [CSV](http://en.wikipedia.org/wiki/Comma-separated_values) | `csv` | `CSV` | `CSV` | Comma separated scalar values |
| [GNUplot](www.gnuplot.info)  | `gnuplot`  | `GNUPlot` | `GNUPlot` | Only support scalar outputs |
| Checkpoint | `checkpoint`  | `Checkpoint` | `Checkpoint` | MOOSE internal format used for restart and recovery |
| Solution History | `solution_history`  | `SolutionHistory`  | `SolutionHistory` | MOOSE internal format used for writing solution history |

## Creating Custom Output Object

- When creating a new output object, the new object must inherit from one of two template classes: `BasicOutput` or `AdvancedOutput`.
- For either base class, the template parameter should be one of the following four output classes:

| Base Class | Description |
| :- | :- |
| `Output` | the most general output base class, this should be used for simple output classes that require very little control over execution, see [`MaterialPropertyDebugOutput`](http://www.mooseframework.com/docs/doxygen/moose/classMaterialPropertyDebugOutput.html) |
| `PetscOutput` | provides the ability to execute output calls on linear and nonlinear residual calculations. |
| `FileOutput` | adds the basic functions and parameters need to write to a file. |
| `OversampleOutput` | adds the ability to utilize an oversampled mesh for outputting. |

*Note, the four classes listed above inherit from each other, so `FileOutput` is a `PetscOutput`, see the inheritance diagram for a visual representation: [Outputs](http://www.mooseframework.com/docs/doxygen/moose/classOutput.html).*

## Creating a BasicOutput

Basic output objects are designed for simple output cases that perform a single output task and do not require control over individual types of output.

- When creating a bsaic output object, the user must override a single virtual method: `output(const OutputExecFlagType & type)`.
- This method should perform all the necessary commands to perform the output.
- The `OutputExecFlagType` specifies what flag the output is currently being called with (e.g. "initial"). This type is a proper Enum and the possible values are listed in `include/base/Moose.h`.

## Creating an AdvancedOutput

Advanced output objects provide additional control over various types (e.g. Postprocessors).

- When creating an advanced output object a call to the static method `enableOutputTypes` must exist in the new objects `validParams` method that indicates which types of outputs the new object will be responsible for outputting. For example, the Exodus output `validParams` method includes:

```cpp
InputParameters params = validParams<AdvancedOutput<OversampleOutput> >();
params += AdvancedOutput<OversampleOutput>::enableOutputTypes("nodal elemental scalar postprocessor input");
```

Each of the keyword listed in this method enables a call to the associated ouput method, ad detailed in the following table.

| Enable Keyword         | Associated Method |
| :-                     | :-                |
| "nodal"                | `outputNodalVariables` |
| "elemental"            | `outputElementalVariables` |
| "scalar"               | `outputScalarVariables` |
| "postprocessor"        | `outputPostprocessors ` |
| "vector_postprocessor" | `outputVectorPostprocessors` |
| "input"                | `outputInput` |
| "system_information"   | `outputSystemInformation ` |

These methods are the virtual methods that must be overloaded in the custom output object.

For example, if "nodal" output is enabled the `outputNodalVariables` should be overloaded. Each of these methods includes the `OutputExecFlagType` as in input variable to the method.

## Further Output documentation

!syntax list /Outputs objects=True actions=False subsystems=False

!syntax list /Outputs objects=False actions=False subsystems=True

!syntax list /Outputs objects=False actions=True subsystems=False
