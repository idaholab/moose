# NEML2 syntax

!if! function=hasCapability('neml2')

The `[NEML2]` block in the MOOSE input file is the entry point for defining NEML2 material model(s). All parameters are listed at [the bottom of the page](syntax/NEML2/index.md#syntax-list).

!alert note
This page is +user-facing+, i.e., the [Input file syntax](syntax/NEML2/index.md#input-syntax) section explains how to write the MOOSE input file in order to connect MOOSE to NEML2 for material modeling. +For developers+, please refer to the [NEML2Action](NEML2Action.md) and [NEML2ActionCommon](NEML2ActionCommon.md) pages regarding what objects are constructed by the underlying Action.

## Input File Syntax id=input-syntax

The basic structure of the `[NEML2]` block is shown below.

!listing test/tests/neml2/custom_model.i block=NEML2

The `[NEML2]` block has two parts:

1. A common area directly underneath the `[NEML2]` block.
2. Nested sub-blocks.

In the above example, there is only one sub-block named `[all]`. However, there can be as many sub-blocks as appropriate. Most of the parameters specified in the common area are also applied to each sub-block. In the case where a parameter is defined both in the common area and under a sub-block, the parameter defined under the sub-block takes precedence.

Sub-blocks are used to specify multiple NEML2 material models used in the same simulation. The following example demonstrates the use of sub-blocks to specify two NEML2 material models on two different subdomains.

!listing test/tests/neml2/blocks_different_model.i block=NEML2

In each sub-block, there are a total of 6 groups of parameters that can be specified:

1. Configuration of the model
2. Transfer of input variables
3. Transfer of model parameters
4. Transfer of output variables
5. Transfer of derivatives (of output variables w.r.t. input variables)
6. Transfer of derivatives (of output variables w.r.t. model parameters)

The configuration of model is controlled by parameters such as [!param](/NEML2/model), [!param](/NEML2/verbose), [!param](/NEML2/device), etc., each of which is explained in the syntax documentation at the bottom of the page.

[!param](/NEML2/input_types) is a list of enums denoting the type of the MOOSE data structure used to hold the input variables. The following enums are supported

- `TIME`: Simulation time.
- `SCALAR`: The input variables are retrieved from a scalar variable and broadcast to all quadrature points.
- `FUNCTION`: The input variables are retrieved from a function evaluated at each quadrature points.
- `VARIABLE`: The input variables are retrieved from (auxiliary) variables interpolated at each quadrature point.
- `MATERIAL`: The input variables are retrieved from material properties stored at each quadrature point.

All NEML2 input variables are automatically retrieved from the host MOOSE simulation. Quantities with the same name as each input variable are retrieved. An error is raised if ambiguity exists, in which case [!param](/NEML2/input_types) and [!param](/NEML2/inputs) can be used to explicitly specify the type of quantities to be retrieved.

All NEML2 output variables are retrieved and stored as MOOSE material properties after each evaluation, unless [!param](/NEML2/auto_output) is set to `false`.

For stateful variables, i.e., input variables needing values from previous time steps (usually with suffix `~N` with `N` being the number of steps backward in time), the corresponding MOOSE quantities from previous time steps are automatically retrieved. The advance of stateful variables is managed by the MOOSE native material system, unless [!param](/NEML2/manage_state_advance) is set to true, in which case NEML2 handles the storage and advance of stateful variables. Note that currently `manage_state_advance = true` is not compatible with mesh change events.

It is worth noting that for [!param](/NEML2/derivatives) and [!param](/NEML2/parameter_derivatives), a pair of names must be specified for each entry. The first name in the pair denotes the quantity (NEML2 output variable) to take derivative of, and the second name in the pair denotes the quantity (NEML2 input variable or model parameter) to take derivative with respect to. Pairs are delimited by `;`.

## Inspect NEML2 information

The command-line option `--parse-neml2-only` can be used to inspect the NEML2 material model *without* running the entire simulation, i.e.

```bash
my-app-opt -i input.i --parse-neml2-only
```

## Work scheduling and dispatching

When the number of material updates (i.e., number of quadrature points) gets large, it is ideal to utilize and coordinate work among multiple devices, e.g., CPUs, GPUs, etc. in a heterogeneous computing environment. This can be achieved using NEML2's work schedulers.

NEML2 offers a variety of different work scheduling strategies, the two most commonly used strategies are defined by
- SimpleScheduler: Dispatch work to a single device.
- StaticHybridScheduler: Coordinate work scheduling and dispatching among multiple devices.

!alert tip
For more information on various types of schedulers and dispatchers, please refer to the [NEML2 documentation](https://applied-material-modeling.github.io/neml2/system-schedulers.html).

The schedulers are defined in the NEML2 input file, and can be selected using the [!param](/NEML2/scheduler) parameter. On top of that, the boolean parameter [!param](/NEML2/async_dispatch) can be used to control whether work is dispatched asynchronously.

!syntax parameters /NEML2/NEML2ActionCommon id=syntax-list
                                            heading=Common parameters
                                            heading-level=2

!syntax parameters /NEML2/NEML2Action id=syntax-list
                                            heading=Sub-block parameters
                                            heading-level=2

!if-end!

!else

!include neml2/neml2_warning.md
