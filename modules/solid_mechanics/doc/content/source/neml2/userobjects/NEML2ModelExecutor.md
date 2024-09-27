# NEML2ModelExecutor

!syntax description /UserObjects/NEML2ModelExecutor

!alert note
Users are +NOT+ expected to directly use this object in an input file. Instead, it is always recommended to use the [NEML2 action](syntax/NEML2/index.md).

## Description

This object uses the specified NEML2 material model to perform mesh-wise (or subdomain-wise) batched material update.

Each NEML2 model +input variable+ is gathered from MOOSE by a `MOOSEToNEML2` user object (gatherer) given in [!param](/UserObjects/NEML2ModelExecutor/gatherers). Optionally, NEML2 model +parameters+ can also be gathered from MOOSE by gatherers given in [!param](/UserObjects/NEML2ModelExecutor/param_gatherers).

Currently, three types of gatherers are available:

- [MOOSEMaterialPropertyToNEML2](MOOSEMaterialPropertyToNEML2.md) gathers material property stored at each quadrature point.
- [MOOSEVariableToNEML2](MOOSEVariableToNEML2.md) gathers (auxiliary) variables interpolated at quadrature points.
- [MOOSEPostprocessorToNEML2](MOOSEPostprocessorToNEML2.md) gathers postprocessor value broadcast to all quadrature points.

Each model +output+ and its +derivatives+ with respect to input variables and model parameters can be retireved by a [NEML2ToMOOSEMaterialProperty](NEML2ToMOOSEMaterialProperty.md) material object.

## NEML2 model execution

The actual execution of the NEML2 model takes place in the `execute()` method. The model execution involves five steps:

1. Re-allocate the model, if necessary
2. Fill out model input variables and parameters
3. Apply the predictor
4. Solve, i.e., perform the material update
5. Extract model output variables and their derivatives

Note that the model is only re-allocated when the gathered batch size and the model's batch size do not match, which could happen

- Before the very first material update;
- After a mesh-change event which results in a change in the number of quadrature points in the operating subdomain.

!syntax parameters /UserObjects/NEML2ModelExecutor
