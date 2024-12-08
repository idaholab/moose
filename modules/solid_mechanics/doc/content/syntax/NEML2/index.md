# NEML2 syntax

The `[NEML2]` block in the MOOSE input file is the entry point for defining NEML2 material model(s). All parameters are listed at [the bottom of the page](syntax/NEML2/index.md#syntax-list).

!alert note
This page is user-facing, i.e., the [Input file syntax](syntax/NEML2/index.md#input-syntax) section explains how to write the MOOSE input file in order to connect MOOSE to NEML2 for material modeling.

!alert note
For developers, please refer to the [NEML2Action](NEML2Action.md) and [NEML2ActionCommon](NEML2ActionCommon.md) pages regarding what objects are constructed by the underlying Action.

## Input File Syntax id=input-syntax

The basic structure of the `[NEML2]` block is shown below.

!listing test/tests/neml2/elasticity.i block=NEML2

The `[NEML2]` block has two parts:

1. A common area directly underneath the `[NEML2]` block.
2. Nested sub-blocks.

In the above example, there is only one sub-block named `[all]`. However, there can be as many sub-blocks as appropriate. Most of the parameters specified in the common area are also applied to each sub-block. In the case where a parameter is defined both in the common area and under a sub-block, the parameter defined under the sub-block overrides.

!alert note
The only exception to the above rule is the [!param](/NEML2/input) parameter which can only be specified in the common area.

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

The other 5 groups of parameters are all related to data transfer between MOOSE and NEML2. The 2nd and the 3rd groups of parameters correspond to the transfer of data +from MOOSE to NEML2+. The 4th, 5th and the 6th groups of parameters correspond to the transfer of data +from NEML2 to MOOSE+.

Each group has three parameters in the following form:

- `moose_<*>_types`: List of types denoting the type of the MOOSE data structure.
- `moose_<*>s`: Names of quantities to be transferred from/to MOOSE.
- `neml2_<*>s`: Names of quantities to be transferred from/to NEML2.

where `<*>` are placeholders representing the data being transferred, e.g., `input`, `parameter`, `output`, `derivative`, `parameter_derivative`. Using `input` as an example, the three parameters are

- `moose_input_types`
- `moose_inputs`
- `neml2_inputs`

The length of the three lists must be the same. [!param](/NEML2/moose_input_types) is a list of enums denoting the type of the MOOSE data structure used to hold the input variables. The following enums are supported

- `MATERIAL`: The input variables are retrieved from material properties stored at each quadrature point.
- `VARIABLE`: The input variables are retrieved from (auxiliary) variables interpolated at each quadrature point.
- `POSTPROCESSOR`: The input variables are retrieved from a postprocessor and broadcast to all quadrature points.

Currently, for the groups of parameters that control the data transfer from NEML2 to MOOSE, only the `MATERIAL` is supported, i.e., NEML2 output variables and derivatives can only be transferred to MOOSE material properties.

It is worth noting that for [!param](/NEML2/neml2_derivatives) and [!param](/NEML2/neml2_parameter_derivatives), a pair of names must be specified for each entry. The first name in the pair denotes the quantity (NEML2 output variable) to take derivative of, and the second name in the pair denotes the quantity (NEML2 input variable or model parameter) to take derivative with respect to.

## Inspect NEML2 information

The MOOSE solid-mechanics module also provides a command-line option to inspect the NEML2 material model *without* running the entire simulation. This is achieved using the `--parse-neml2-only` command-line argument, i.e.

```bash
solid_mechanics-opt -i input.i --parse-neml2-only
```

Below is an example output

```
*** NEML2 MATERIAL MODEL SUMMARY BEGIN ***
Input file location:       modules/solid_mechanics/test/tests/neml2/viscoplasticity_chaboche.i:34
Input file path:           NEML2/all
Material model Name:       model
Evaluation device:         cpu
Automatic differentiation: Disabled
Material model structure:
------------------------------------------------------------------
|   Type    |         Name          | Tensor type | Storage size |
------------------------------------------------------------------
| input     | forces/E              | SR2         |            6 |
| input     | forces/t              | Scalar      |            1 |
| input     | old_forces/E          | SR2         |            6 |
| input     | old_forces/t          | Scalar      |            1 |
| input     | old_state/S           | SR2         |            6 |
| input     | old_state/internal/X1 | SR2         |            6 |
| input     | old_state/internal/X2 | SR2         |            6 |
| input     | old_state/internal/ep | Scalar      |            1 |
| input     | state/S               | SR2         |            6 |
| input     | state/internal/X1     | SR2         |            6 |
| input     | state/internal/X2     | SR2         |            6 |
| input     | state/internal/ep     | Scalar      |            1 |
| output    | state/S               | SR2         |            6 |
| output    | state/internal/X1     | SR2         |            6 |
| output    | state/internal/X2     | SR2         |            6 |
| output    | state/internal/ep     | Scalar      |            1 |
| parameter | X1rate.A              | Scalar      |            1 |
| parameter | X1rate.C              | Scalar      |            1 |
| parameter | X1rate.a              | Scalar      |            1 |
| parameter | X1rate.g              | Scalar      |            1 |
| parameter | X2rate.A              | Scalar      |            1 |
| parameter | X2rate.C              | Scalar      |            1 |
| parameter | X2rate.a              | Scalar      |            1 |
| parameter | X2rate.g              | Scalar      |            1 |
| parameter | elasticity.E          | Scalar      |            1 |
| parameter | elasticity.nu         | Scalar      |            1 |
| parameter | flow_rate.eta         | Scalar      |            1 |
| parameter | flow_rate.n           | Scalar      |            1 |
| parameter | isoharden.R           | Scalar      |            1 |
| parameter | isoharden.d           | Scalar      |            1 |
| parameter | yield.sy              | Scalar      |            1 |
| buffer    | Eerate.c              | Tensor      |            2 |
| buffer    | kinharden.c           | Tensor      |            1 |
| buffer    | overstress.c          | Tensor      |            2 |
------------------------------------------------------------------
*** NEML2 MATERIAL MODEL SUMMARY END ***
```

!syntax parameters /NEML2/NEML2ActionCommon id=syntax-list
                                            heading=Common parameters
                                            heading-level=2

!syntax parameters /NEML2/NEML2Action id=syntax-list
                                            heading=Sub-block parameters
                                            heading-level=2
