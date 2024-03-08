# NEML2 syntax

The `NEML2` block is used to construct a set of material and user objects so that MOOSE can "outsource" the material update to NEML2. Different objects get created depending on the operation mode (explained below). They are summarized in the following table for convenience:

| Object                        | Type       | Operation mode | Description                                                                                  |
| ----------------------------- | ---------- | -------------- | :------------------------------------------------------------------------------------------- |
| CauchyStressFromNEML2         | Material   | ELEMENT        | Perform element-wise batched material update                                                 |
| CauchyStressFromNEML2UO       | UserObject | ALL            | Perform mesh-wise batched material update                                                    |
| CauchyStressFromNEML2Receiver | Material   | ALL            | Assign the outputs of the batched material update into the corresponding material properties |

## Example Input Syntax

!listing test/tests/neml2/fem.i block=NEML2

The field [!param](/NEML2/NEML2Action/input) specifies the relative path to the NEML2 input file. The field [!param](/NEML2/NEML2Action/model) tells MOOSE which material model to import from the NEML2 input file. The field [!param](/NEML2/NEML2Action/device) specifies where to evaluate the NEML2 model, e.g., CPU or CUDA. The parameter [!param](/NEML2/NEML2Action/mode) determines the mode of operation for NEML2, and it is important to understand the differences between the modes in order to use NEML2 most efficiently. Each mode is discussed below in detail.

## Operation Mode: PARSE_ONLY

In this mode, the NEML2 input file is parsed, and all the objects specified in the NEML2 input file are created. No MOOSE objects are being created in this mode of operation. This mode offers the highest level of flexibility, and requires the most amount of effort to set up the necessary MOOSE objects.

## Operation Mode: ELEMENT

In this mode, a regular MOOSE material object [`CauchyStressFromNEML2`](CauchyStressFromNEML2.md optional=True) is constructed to perform the constitutive update. On each element, the material

1. loops through all the quadrature points and gathers all the necessary input variables into a batched input vector with batch size equal to the number of quadrature points;
2. calls the NEML2 material model to compute the batched output given the batched input;
3. loops through each quadrature point and assigns the output variables to the corresponding MOOSE material properties.

## Operation Mode: ALL

In this mode, a user object [`CauchyStressFromNEML2UO`](CauchyStressFromNEML2UO.md optional=True) and a regular MOOSE material object [`CauchyStressFromNEML2Receiver`](CauchyStressFromNEML2Receiver.md optional=True) are created. During each residual evaluation,

1. The user object gathers all input variables from all quadrature points into a batched input vector with batch size equal to the total number of quadrature points on the mesh.
2. The user object calls the NEML2 material model to compute the batched output given the batched input.
3. The material object retrieves the batched output from the user object and assigns the output variables to the corresponding MOOSE material properties.

## Efficiency Considerations

As discussed earlier, the ELEMENT mode and the ALL mode produce input vectors with different batch sizes. NEML2 handles the threading and vectorization of the batched evaluation. When the batch size is large (i.e. in the ALL mode), it allows for a potentially more aggressive use of the available computing resource, and GPUs can make the evaluation a lot faster relying on massive vectorization.

## Verbosity

Only two verbosity levels are currently implemented, e.g., `verbose = true` and `verbose = false`. In the verbose mode, additional information about the NEML2 material model is printed to the console. An example output is listed below

```text
*** BEGIN NEML2 INFO ***

Input:
----------------------------------------
|       Variable        | Storage size |
----------------------------------------
| forces/E              |            6 |
| forces/t              |            1 |
| old_forces/E          |            6 |
| old_forces/t          |            1 |
| old_state/S           |            6 |
| old_state/internal/Kp |            6 |
| old_state/internal/ep |            1 |
| state/S               |            6 |
| state/internal/Kp     |            6 |
| state/internal/ep     |            1 |
----------------------------------------

Output:
------------------------------------
|     Variable      | Storage size |
------------------------------------
| state/S           |            6 |
| state/internal/Kp |            6 |
| state/internal/ep |            1 |
------------------------------------

Parameters:
---------------------------------------------------------
|               Parameter               | Requires grad |
---------------------------------------------------------
| implicit_rate.kinharden.H             | False         |
| implicit_rate.isoharden.K             | False         |
| implicit_rate.normality.flow.yield.sy | False         |
| implicit_rate.yield.sy                | False         |
| implicit_rate.flow_rate.eta           | False         |
| implicit_rate.flow_rate.n             | False         |
| implicit_rate.elasticity.E            | False         |
| implicit_rate.elasticity.nu           | False         |
---------------------------------------------------------

*** END NEML2 INFO ***
```

## Inspect NEML2 information

The MOOSE solid-mechanics module also provides a command-line option to inspect the NEML2 material model _without_ running the entire simulation. This is achieved using the `--parse-neml2-only` command-line argument, i.e.

```bash
solid_mechanics-opt -i input.i --parse-neml2-only
```

!syntax parameters /NEML2/NEML2Action
