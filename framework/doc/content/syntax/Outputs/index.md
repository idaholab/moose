# Output System

The output system is designed to be just like any other system in MOOSE: modular and expandable.

## Short-cut Syntax

To enable output in an application it must contain an `[Outputs]` block.  The simplest method for
enabling output is to utilize the short-cut syntax as shown below, which enables
[outputs/Exodus.md] output for writing spatial data. A complete list of output types and the
associated short-cut syntax for the framework is included in [output-types].

!listing id=output-shortcut caption=Example of short-cut syntax in the Outputs block.
[Outputs]
  exodus = true
[]

Developers may refer to [CommonOutputAction.md] for more information about implementing short-cut
syntax for new Output types.

## Block Syntax

To take full advantage of the output system, the use of sub-blocks is required. The block listed
below is nearly identical, with respect to the internal implementation, to [output-shortcut], including
the block name (i.e., the short-cut syntax builds this exact block). The one difference between
the short-cut and block syntax is the default output filename assigned

!listing id=output-block caption=Example of full block syntax in the Outputs block.
[Outputs]
  [exodus]
    type = Exodus
  []
[]

## Filenames

The resulting filenames produced by different syntax is important. The default naming scheme for
output files utilizes the input file name (e.g., input.i) with a suffix that differs depending on
how the output is defined:

- An "_out" suffix is used for `Outputs` created using the short-cut syntax.
- The sub-block syntax uses the actual sub-block name as the suffix.

For example, if the input file (input.i) contained the following `[Outputs]` block, two files would be created: input_out.e and input_other.e.

!listing id=output-names caption=Example of output filenames, assuming an input file name of 'input.i'.
[Outputs]
  console = true
  exodus = true    # creates input_out.e
  [other]        # creates input_other.e
     type = Exodus
  []
[]

Note, the use of "file_base" anywhere in the `[Outputs]` block disables all default naming behavior.

## Available Output Types

[output-types] provides a list of the most common output types, including the short-cut syntax
as well as the type to be used when creating a sub-block. A complete list of all available
output objects is provided below.

!table id=output-types caption=List of common output objects included in core MOOSE framework
!include output_types.md

!syntax list /Outputs actions=False subsystems=False id=output-objects heading=Complete list of available Output objects. heading-level=3

## Multiple Output Blocks

It is possible to create multiple output objects for outputting:

- at specific time or timestep intervals,
- custom subsets of variables, and
- to various file types.
- Supports common parameters and sub-blocks.

!listing id=multiple-outputs caption=Example of creating multiple Output objects.
[Outputs]
  vtk = true
  [console]
    type = Console
    perf_log = true
    max_rows = true
  []
  [exodus]
    type = Exodus
    execute_on = 'timestep_end'
  []
  [exodus_displaced]
    type = Exodus
    file_base = displaced
    use_displaced = true
    interval = 3
  []
[]

## Common Parameters

The Outputs block supports common parameters. For example, "execute_on" may be specified outside of
individual sub-blocks, indicating that all sub-blocks should output the initial condition, for
example.

If within a sub-block the parameter is given a different value, the sub-block parameter takes
precedence.

The input file snippet below demonstrate the usage of a common values as well as the use of
multiple output blocks.

!alert note title=[Console objects](Console.md) are a special case.
`Console`-based outputs have special inheritance of common parameters: the `execute_on` parameter,
as detailed below, does not get inherited.

!listing id=common-output-params caption=Example use of common parameters in the Outputs block.
[Outputs]
  execute_on = 'timestep_end' # disable the output of initial condition
  interval = 2                # only output every 2 timesteps
  vtk = true                  # output VTK file
  print_perf_log = true       # enable the performance log
  [initial]
    type = Exodus
    execute_on = 'initial'    # this ExodusII file will contain ONLY the initial condition
  []
  [displaced]
    type = Exodus
    use_displaced = true
    interval = 3              # this ExodusII will only output every third time step
  []
[]

Developers may refer to [CommonOutputAction.md] for more information about implementing new
common output parameters.

## Controlling output frequency

Several parameters are available common to all output objects to control the frequency with which
output occurs.

- `interval = N` will cause output objects to output only every _Nth_ simulation step
- if `start_time` and/or `end_time` are specified, outputs will only happen after the given start time and/or before the given end time
- `sync_times = 't1 t2 t3 ... tN` will cause MOOSE to hit each listed simulation time _t1 .. tN_ exactly. With `sync_only = true` outputs will _only_ happen at the specified sync times.
- `minimum_time_interval = dt` will suppress any output if the previous output happened at an earlier time that is more recent than _dt_ time units ago (specifically this means that outputs during linear iterations are not suppressed, as they are happening at the _same_ simulation time, and output from failed, cut steps do not lead to output suppression in repeated steps as they happened in the future)

## Outputs and execute_on

Outputs utilize the "execute_on" parameter like other systems in MOOSE, by default:

- All objects have `execute_on = 'initial timestep_end'`, with `Console` objects being the exception.
- `Console` objects append `'initial timestep_begin linear nonlinear failed'` to the "execute_on"
  settings at the common level.

A list of available "execute_on" flags for Output objects is provided in [output-execute-on] and
a convenience parameter, "additional_execute_on", allows appending flags to the existing
"execute_on" flags.

When debugging output, use the `--show-outputs` flag when executing your application. This will add a
section near the top of the simulation that shows the settings being used for each output object.

- The toggles shown below provide additional operate by appending to the "execute_on" flags.
- Currently, the following output execution times are available:

!table id=output-execute-on caption=List of available 'execute_on' values for Output objects.
| Text Flag | Description |
| :- | :- |
| "none"           | disables the output |
| "initial"        | executes the output on the initial condition (on by default) |
| "linear"         | executes the output on each linear iteration |
| "nonlinear"      | executes the output on each non-linear iteration |
| "timestep_end"   | calls the output method at the end of the timestep (on by default) |
| "timestep_begin" | executes the output method at the beginning of the timestep |
| "final"          | calls the output method on the final timestep |
| "failed"         | executes the output method when the solution fails |

As detailed in the [#custom-output] section, there are two types of outputs
classes: `BasicOutput` and `AdvancedOutput`. Advanced outputs have additional control beyond
`execute_on`, as detailed in the table below.

| Input Parameter | Method Controlled |
| :- | :- |
| execute_postprocessors_on | `outputPostprocessors` |
| execute_vector_postprocessors_on | `outputVectorPostprocessors` |
| execute_elemental_on | `outputElementalVariables` |
| execute_nodal_on | `outputNodalVariables` |
| execute_scalar_on | `outputScalarVariables` |
| execute_system_information_on | `outputSystemInformation` |
| execute_input_on | `outputInput` |

Each of the output controls accept the same output execution flags that `execute_on` utilizes. In
`AdvancedOutput` objects, the `execute_on` settings are used to set the defaults for each of the
output type controls. For example, setting `execute_on = 'initial timestep_end'` causes all of the
output types (e.g., postprocessors, scalars, etc.) to execute on each timestep and the initial
condition.

## Mesh Displacements and Higher Order Meshes

The displaced mesh may be output by setting 'use_displaced = true' within your output sub-blocks. To
output both the original and displaced mesh an additional output block is required.

!alert warning title=Be careful when outputting a displaced mesh!
In general, it is not necessary to output the displace mesh, since most visualization tools
automatically apply displacements. Because of this, writing the displaced mesh can result in
visualizations that apply displacements multiple times.

If a simulation is using a higher order mesh, oversampling may be required to provide an accurate
representation of the finite element solution. Visualization tools generally perform linear
interpolation between data, regardless of the mesh order. Oversampling, which is controlled by
setting the "refinements" parameter, will evaluate the finite element solution on a uniformly refined
mesh during output to provide a improved view of the existing solution.

!listing id=disp-oversample caption=Example showing output of displaced mesh and oversampling.
[Outputs]
  console = true
  exodus = true
  [oversample]
     type = Exodus
     refinements = 2
     use_displaced = true
  []
[]

## In-Block Output Control

For certain objects it is possible to control output within the block itself, these objects include:
Postprocessors, VectorPostprocessors, Indicators, Markers, Variables, and AuxVariables. For example,
consider the following input file that has three output objects defined and a single postprocessor as well.

```text
[Postprocessors]
  [pp]
    type = EmptyPostprocessor
    outputs = 'csv'
  []
[]

[Outputs]
  exodus = true
  csv = true
  [vtk]
    type = VTK
    interval = 2
  []
[]
```

The outputs that should include the postprocessor value may be listed in the "outputs" parameter.

The "outputs" parameter limits the defined postprocessor to output only to the `csv` output
object. This highlights the need to understand the naming convention utilized by the short-cut
syntax. It is also possible to remove the postprocessor from all outputs by specifying
"outputs = none".

## Non-linear/Linear Residual Output

Any object inheriting from PetscOutput has the ability to output data on non-linear and/or linear
iterations. To enable this add "nonlinear" and/or "linear" to the "execute_on" input parameter.

```text
[Outputs]
  [exodus]
    type = Exodus
    execute_on = 'timestep_end linear nonlinear'
  []
[]
```

When outputting nonlinear/linear iterations the time is changes from the actual simulation time by
creating pseudo time steps.  For example, if the `[Outputs]` block above was associated with a
simulation that was taking a time step of 0.1 the resulting output times would use the following
convention:

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

## Creating Custom Output Object id=custom-output

When creating a new output object, the new object must inherit from one of two templated base
classes: `BasicOutput` or `AdvancedOutput`.

For either base class, the template parameter should be one of the following four output classes:

| Base Class | Description |
| :- | :- |
| `Output` | the most general output base class, this should be used for simple output classes that require very little control over execution, see [`MaterialPropertyDebugOutput`](http://www.mooseframework.com/docs/doxygen/moose/classMaterialPropertyDebugOutput.html) |
| `PetscOutput` | provides the ability to execute output calls on linear and nonlinear residual calculations. |
| `FileOutput` | adds the basic functions and parameters need to write to a file. |
| `OversampleOutput` | adds the ability to utilize an oversampled mesh for outputting. |

Note, the four classes listed above inherit from each other, so `FileOutput` is a `PetscOutput`, see
the inheritance diagram for a visual representation:
[`Output`](http://www.mooseframework.com/docs/doxygen/moose/classOutput.html).

## Creating a BasicOutput

Basic output objects are designed for simple output cases that perform a single output task and do
not require control over individual types of output.

- When creating a basic output object, the user must override a single virtual method: `output(const
  OutputExecFlagType & type)`.
- This method should perform all the necessary commands to perform the output.
- The `OutputExecFlagType` specifies what execute flag the output is currently being called
  with(e.g., "initial"). This type is a proper Enum and the possible values are listed in
  `include/base/Moose.h`.

## Creating an AdvancedOutput id=advanced-output

Advanced output objects provide additional control over various output types (e.g., postprocessors).
When creating an advanced output object a call to the static method `enableOutputTypes` must exist in
the new objects `validParams` method that indicates which types of outputs the new object will be
responsible for outputting. For example, the Exodus output `validParams` method includes:

```cpp
InputParameters params = AdvancedOutput<OversampleOutput>::validParams();
params += AdvancedOutput<OversampleOutput>::enableOutputTypes("nodal elemental scalar
                                                              postprocessor input");
```

Each of the keywords listed in this method enable a call to the associated output method, as detailed
in the following table.

| Enable Keyword         | Associated Method |
| :-                     | :-                |
| "nodal"                | `outputNodalVariables` |
| "elemental"            | `outputElementalVariables` |
| "scalar"               | `outputScalarVariables` |
| "postprocessor"        | `outputPostprocessors` |
| "vector_postprocessor" | `outputVectorPostprocessors` |
| "input"                | `outputInput` |
| "system_information"   | `outputSystemInformation` |

These methods are the virtual methods that must be overloaded in the custom output object. For
example, if "nodal" output in enabled the `outputNodalVariables` should be overloaded. Each of these
methods includes the `OutputExecFlagType` as an input variable to the method.
