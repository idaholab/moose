<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# Postprocessors System

## Overview

- A `Postprocessor` is a "reduction" or "aggregation" calculation based on the solution variables which results in a +single+ scalar value.
- `Postprocessors` are computed at the times specified by the `execute_on` option in the input file:

  - `execute_on = timestep_end` 
  - `execute_on = linear`
  - `execute_on = nonlinear`
  - `execute_on = timestep_begin`
  - `execute_on = custom`
  
- They can be restricted to specific blocks, sidesets, and nodesets in your domain.

## Types of Postprocessors

- Element

  - Operate om each element.
  - Can be restricted to subdomains by specifying one or more `block` ids.
  - Inherit from `ElementPostprocessor`.

- Nodal

  - Operate on each node
  - Can be restricted to nodesets by specifying one or more `boundary` ids.
  - Inherit from `NodalPostprocessor`.

- Side

  - Operate on boudaries.
  - Must specify one or more `boundary` ids to compute on.
  - Inherit from `SidePostprocessor`.

- General

  - Does whatever it wants.
  - Inherit from `GeneralPostprocessor`

## Postprocessor Anatomy

`Postprocessor` virtual functions for implementing your aggregation operation:

- `void initialize()` : Clear or initialize your data structure before execution.
- `void execute()` : Called on each geometric entity for the type of this `Postprocessor`.
- `void threadJoin(const UserObject $ uo)

  - Aggregation across threads
  - Called to join the passed in `Postprocessor` with this `Postprocessor`.
  - You have the local access to the data structures in both `Postprocessors`.

- `void finalize()`

  - Aggregation across MPI.
  - One of the only places in MOOSE where you might need to use MPI!
  - Several Aggregation routines are available in libMesh's `parallel.h` file.

- `Real getValue()` : Retrieve the final scalar value.

## Helpful Aggregation Routines

If the Postprocessor you are creating has custm data (i.e. you are accumulating or computing a member variable inside your Postprocessor) you will need to ensure that the value is communicated properly in (both MPI and thread-based) parallel simulations.

For MPI we provide several utility routines to perform common aggregation operations:

- MOOSE convenience functions:

  - `gatherSum(scalar)` -- returns the sum of `scalar` across all processors.
  - `gatherMin(scalar)` -- returns the min of `scalar` from all processors.
  - `gatherMax(scalar)` -- returns the max of `scalar` from all processors.
  - `gatherProxyValueMax(scalar, proxy)` -- returns `proxy` based on max `scalar`.

- LibMesh convenience functions (from `parallel.h`):

  - `_communicator.max(...)`
  - `_communicator.sum(...)`
  - `_communicator.min(...)`
  - `_communicator.gather(...)`
  - `_communicator.send(...)`
  - `_communicator.receive(...)`
  - `_communicator.set_union(...)`
  
- LibMesh functions work with a wide variety of types (scalars, vectors, sets, maps, . . .)

```cpp
    void
    PPSum::finalize()
    {
      gatherSum(_total_value);
    }
```

## ThreadJoin (Advanced)

- You do not need to implement this function to run in parallel. Start with `finalize() and use MPI only.
- You generally need to cast the base class reference to the current type so that you can access the data stucture within.
- Use to perform custom aggregation operations for yout class.

```cpp
    void
    PPSum::threadJoin(const UserObject & y)
    {
      // Cast UserObject into a PPSum object so that we can access member variables
      const PPSum & pps = static_cast<const PPSum &>(y);

      _total_value += pps._total_value;
    }
```

## Postprocessor Types

- A few types of built in `Postprocessors`:

  - `ElementIntegral`, `ElementAverageValue`
  - `SideIntegral`, `SideAverageValue`
  - `ElementL2Error`, `ElementH1Error`
  
- Each of these classes can be extended via inheritance.
- For instance, if you want the average flux on one side you can inherit from `SideAverageValue` and override `computeQpIntegral()` to compute the flux at every quadrature point.
- For the Element and Side `Postprocessors`, you can use material properties (and `Functions`).
- By default, `Postprocessors` will output to a formatted table on the screen, but they can also write to a CSV or Tecplot file.
- `Postprocessors` can also be written to Exodus files as "global" data.

## Default Postprocessor Values

- It is possible to set default values for `Postprocessors`.
- This allows a `MooseObject` (e.g. `Kernel`) to operate without creating or specifying a `Postprocessor`.
- Within the `validParams()` function for your object, declare a `Postprocessor` parameter with a default value.

```cpp
params.addParam<PostprocessorName>("postprocessor", 1.2345, "Doc String");
```

- When you use the `getPostProcessorValue()` interface, MOOSE provides the user-defined value, or the default if no `Postprocessor` has been specified.

```cpp
const PostprocessorValue & value = getPostprocessorValue("postprocessor");
```

- Additionally, users may supply a real value in the input file in lieu of a `Postprocessor` name.

## Input File Syntax and Output

- `Postprocessors` are declared in the `Postprocessors` block.
- The name of the sub-block (like side_average and integral) is the "name" of the `Postprocessor, amd will be the name of the column in the output.
- Element and Side `Postprocessors` generally take a variable argument to work on, but can also be coupled to other variables in the same way that `Kernels`, `BCs`, etc. can.

```puppet
[Postprocessors]
  [./dofs]
    type = NumDOFs
  [../]
  [./h1_error]
    type = ElementH1Error
    variable = forced
    function = bc_func
  [../]
  [./l2_error]
    type = ElementL2Error
    variable = forced
    function = bc_func
  [../]
[]
```

- Postprocessor results are printed to the screen as well as *.csv and *.e files.

```text
Postprocessor Values:
+----------------+----------------+----------------+----------------+
| time           | dofs           | h1_error       | l2_error       |
+----------------+----------------+----------------+----------------+
|   0.000000e+00 |   9.000000e+00 |   2.224213e+01 |   9.341963e-01 |
|   1.000000e+00 |   9.000000e+00 |   2.224213e+01 |   9.341963e-01 |
|   2.000000e+00 |   2.500000e+01 |   6.351338e+00 |   1.941240e+00 |
|   3.000000e+00 |   8.100000e+01 |   1.983280e+01 |   1.232381e+00 |
|   4.000000e+00 |   2.890000e+02 |   7.790486e+00 |   2.693545e-01 |
|   5.000000e+00 |   1.089000e+03 |   3.995459e+00 |   7.130219e-02 |
|   6.000000e+00 |   4.225000e+03 |   2.010394e+00 |   1.808616e-02 |
|   7.000000e+00 |   1.664100e+04 |   1.006783e+00 |   4.538021e-03 |
+----------------+----------------+----------------+----------------+
```

## Further Postprocessor Documentation

!syntax list /Postprocessors objects=True actions=False subsystems=False

!syntax list /Postprocessors objects=False actions=False subsystems=True

!syntax list /Postprocessors objects=False actions=True subsystems=False

