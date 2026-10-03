# NEML2ModelExecutor

!if! function=hasCapability('neml2')

!syntax description /UserObjects/NEML2ModelExecutor

!alert note
Users are +NOT+ expected to use this object directly in an input file. Use the
[NEML2 action](syntax/NEML2/index.md) instead.

## Description

This object uses the specified NEML2 material model to perform mesh-wise (or subdomain-wise) batched material update.

Each NEML2 model +input variable+ is gathered from MOOSE by a `MOOSEToNEML2`
user object specified in
[!param](/UserObjects/NEML2ModelExecutor/gatherers). NEML2 model +parameters+
can also be gathered from MOOSE by user objects specified in
[!param](/UserObjects/NEML2ModelExecutor/param_gatherers).

Each model +output+ and its +derivatives+ with respect to input variables and
model parameters can be retrieved by a
[NEML2ToMOOSEMaterialProperty](NEML2ToMOOSEMaterialProperty.md) material
object.

## NEML2 model execution

The actual execution of the NEML2 model takes place in the `execute()` method. The model execution involves five steps:

1. Re-allocate the model, if necessary
2. Fill out model input variables and parameters
3. Apply the predictor
4. Solve, i.e., perform the material update
5. Extract model output variables and their derivatives

The model is reallocated only when the gathered batch size does not match the
model batch size. This can happen:

- Before the very first material update;
- After a mesh-change event which results in a change in the number of quadrature points in the operating subdomain.

## Managed State Advance

With `manage_state_advance = true`, state history remains in the NEML2 device
cache rather than making a round trip through MOOSE material properties.
History that is not yet cached is initialized with the live inputs' dynamic
batch shape. It is initialized to zero by default. Full second-order state
variables listed in `identity_seeded_state` are instead initialized to the
second-order identity, which is required for multiplicative quantities such as
a plastic deformation gradient.

Managed state advance requires a fixed mesh, a constant time step, and a
workflow in which accepted state is advanced only at the end of a converged
step. It is intended primarily for explicit dynamics.

!syntax parameters /UserObjects/NEML2ModelExecutor

!if-end!

!else

!include neml2/neml2_warning.md
