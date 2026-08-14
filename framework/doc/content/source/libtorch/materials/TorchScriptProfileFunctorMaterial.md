# TorchScriptProfileFunctorMaterial

!syntax description /FunctorMaterials/TorchScriptProfileFunctorMaterial

## Overview

`TorchScriptProfileFunctorMaterial` evaluates a TorchScript model and converts its output into one
or more spatially varying functor material properties. It is intended for models that predict a
discrete one-dimensional profile, such as a friction coefficient, heat-transfer coefficient, or
closure correction along a channel. The predicted values are linearly interpolated in profile
coordinates and may be consumed by finite-element and finite-volume objects through the MOOSE
functor interface.

The TorchScript module is loaded and evaluated through a
[`TorchScriptUserObject`](../userobjects/TorchScriptUserObject.md). Unlike
[`TorchScriptMaterial`](TorchScriptMaterial.md), which creates conventional quadrature-point
material properties, this object creates `ADReal` functors that can be evaluated using the spatial
arguments supplied by a consuming MOOSE object.

## Model inputs and outputs

Model inputs are supplied using exactly one of the following parameters:

- `input_names`, which obtains the model inputs from scalar postprocessors; or
- `input_values`, which supplies constant model inputs directly in the input file.

For $M$ inputs, the material constructs a tensor of shape `[1, M]`. The tensor scalar type is
selected with `tensor_dtype`.

The TorchScript model may return any of the following shapes:

- `[N]` for one profile containing $N$ stations;
- `[C, N]` for $C$ profiles containing $N$ stations each; or
- `[1, C, N]` with a leading unit batch dimension.

The number of output profiles $C$ must equal the number of entries in `profile_names`, and the
number of stations $N$ must equal the number of entries in `profile_coordinates`. The coordinates
must be finite and strictly increasing. Each row of the normalized `[C, N]` output tensor is paired
with `profile_coordinates` to construct one linearly interpolated functor.

!alert warning title=Profile update and automatic differentiation
The TorchScript model is evaluated once during initial setup. Consequently, the generated profiles
remain fixed during the simulation. Values referenced by `input_names` must be available when
initial setup occurs. The published functors have type `ADReal` for compatibility with AD consumers,
but TorchScript inference itself is not part of the MOOSE automatic-differentiation graph.

## Spatial coordinate mapping

At a physical point $\boldsymbol{x}$, the scalar coordinate used to evaluate the profile is

!equation id=torchscript-profile-coordinate
s(\boldsymbol{x}) =
\frac{(\boldsymbol{x}-\boldsymbol{x}_0)\mathbin{\cdot}\widehat{\boldsymbol{d}}}{L_s},

where $\boldsymbol{x}_0$ is `profile_origin`, $\widehat{\boldsymbol{d}}$ is the normalized
`profile_direction`, and $L_s$ is `coordinate_scale`. The supplied direction therefore does not
need to have unit length, but it must be finite and nonzero. `coordinate_scale` converts mesh
distance along the selected direction into the coordinate system used by `profile_coordinates`.

The `out_of_range_behavior` parameter determines how $s$ values outside the tabulated coordinate
range are handled:

- `error` reports an error;
- `clamp` returns the value at the nearest endpoint; and
- `extrapolate` linearly extrapolates using the nearest two profile stations.

## Example

The following test model maps the two inputs `[a, b]` to two profiles at coordinates `[0, 1, 2]`:

!listing test/tests/functormaterials/torchscript_profile_functor_material/generate_model.py
         language=python

For the input `[10, 5]`, the model returns

!equation
\begin{bmatrix}
10 & 12 & 14 \\
5 & 4 & 3
\end{bmatrix}.

The first row defines `profile_a`, and the second row defines `profile_b`.

The material is configured in the test input as follows:

!listing test/tests/functormaterials/torchscript_profile_functor_material/profile.i
         block=UserObjects

!listing test/tests/functormaterials/torchscript_profile_functor_material/profile.i
         block=FunctorMaterials

With `profile_origin = '2 0 0'`, `profile_direction = '2 0 0'`, and
`coordinate_scale = 2`, a point at $x=3$ has profile coordinate $s=(3-2)/2=0.5$.
Linear interpolation therefore gives `profile_a = 11` and `profile_b = 4.5` at that point.

## Usage notes

- The order of `profile_names` defines the correspondence between tensor rows and generated
  functors.
- The same units must be used for `profile_coordinates` and the mapped coordinate $s$.
- Use `load_during_construction = true` on the `TorchScriptUserObject` so that the module is
  available when this material performs initial setup.
- The saved TorchScript model should be generated with a PyTorch version compatible with the
  LibTorch version linked into MOOSE.

!syntax parameters /FunctorMaterials/TorchScriptProfileFunctorMaterial

!syntax inputs /FunctorMaterials/TorchScriptProfileFunctorMaterial

!syntax children /FunctorMaterials/TorchScriptProfileFunctorMaterial
