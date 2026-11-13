# TorchScript1DProfileFunctorMaterial

!if! function=hasLibtorch()

## Overview

`TorchScript1DProfileFunctorMaterial` evaluates a TorchScript model and converts its output into one
or more spatially varying functor material properties. It is intended for models that predict a
discrete one-dimensional profile, such as a friction coefficient, heat-transfer coefficient, or
closure correction along a channel. The predicted values are linearly interpolated in profile
coordinates and may be consumed by finite-element and finite-volume objects through the MOOSE
functor interface.

The TorchScript module is loaded and evaluated through a
[TorchScriptUserObject.md]. Unlike
[TorchScriptMaterial.md], which creates conventional quadrature-point
material properties, this object creates `Real` functors that can be evaluated using the spatial
arguments supplied by a consuming MOOSE object.

## Model inputs and outputs

Model inputs are supplied using exactly one of the following parameters:

- [!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/input_names), which obtains the
  model inputs from scalar postprocessors; or
- [!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/input_values), which supplies
  constant model inputs directly in the input file.

For $M$ inputs, the material constructs a tensor of shape `[1, M]`. The tensor scalar type is
selected with [!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/tensor_dtype), which
defaults to `float32`.

The TorchScript model may return any of the following shapes:

- `[N]` for one profile containing $N$ stations along a line;
- `[C, N]` for $C$ profiles containing $N$ stations along the line each; or
- `[1, C, N]` with a leading unit batch dimension.

The number of output profiles $C$ must equal the number of entries in
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/profile_names), and the number of
stations $N$ must equal the number of entries in
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/profile_coordinates). The coordinates
must be finite and strictly increasing. Each row of the normalized `[C, N]` output tensor is paired
with [!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/profile_coordinates) to construct
one linearly interpolated functor.

## Update schedule

The TorchScript model is evaluated according to
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/execute_on), which accepts two
flags:

- `INITIAL` (default) evaluates the model once during initial setup, so the generated profiles
  remain fixed for the entire simulation.
- `TIMESTEP_BEGIN` additionally re-runs inference at the start of every time step, so the
  profiles can follow time-varying inputs.

Only these two flags are offered because the model consumes solely the scalar values supplied
through [!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/input_names) or
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/input_values).
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/execute_on) therefore controls
*when* inference re-runs. Between updates, the generated profiles remain fixed.

If an entry in [!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/input_names) is a
postprocessor that depends on the solution, the material reads only that postprocessor's most
recently stored scalar value:

- In a steady simulation, inference occurs before the nonlinear solve. The default `INITIAL`
  evaluation can occur before the input postprocessor has performed its normal `INITIAL`
  execution. Including `TIMESTEP_BEGIN` causes one additional profile update immediately before
  the steady solve, after `INITIAL` postprocessors have executed. This update can therefore use
  postprocessor values computed from the initialized solution, but the resulting profiles remain
  fixed throughout the nonlinear iterations.
- In a transient simulation with `TIMESTEP_BEGIN`, the material rebuilds its profiles before
  postprocessors scheduled on `TIMESTEP_BEGIN` are recomputed. It therefore uses the value stored
  by the input postprocessor at its preceding execution, normally a value computed from the
  previous accepted solution. The profiles then remain fixed throughout the current time-step
  solve. A postprocessor that also executes on `TIMESTEP_BEGIN` is consequently seen with a
  one-step lag.

!alert warning title=Profile availability and automatic differentiation
Values referenced by
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/input_names) must be available when
each profile update occurs. The published functors do not reevaluate the model when the solution
changes or distinguish current and old solution states. They have type `Real`, and TorchScript
inference is performed outside the MOOSE automatic-differentiation graph. Consequently, a
solution-dependent input is treated as a lagged explicit coupling: its dependence on the current
nonlinear solution is not represented in the Jacobian.

## Spatial coordinate mapping

At a physical point $\boldsymbol{x}$, the scalar coordinate used to evaluate the profile is

!equation id=torchscript-profile-coordinate
s(\boldsymbol{x}) =
\frac{(\boldsymbol{x}-\boldsymbol{x}_0)\mathbin{\cdot}\widehat{\boldsymbol{d}}}{L_s},

where $\boldsymbol{x}_0$ is
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/profile_origin),
$\widehat{\boldsymbol{d}}$ is the normalized
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/profile_direction), and $L_s$ is
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/coordinate_scale). The supplied
direction therefore does not need to have unit length, but it must be finite and nonzero.
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/coordinate_scale) converts mesh
distance along the selected direction into the coordinate system used by
[!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/profile_coordinates).

The [!param](/FunctorMaterials/TorchScript1DProfileFunctorMaterial/out_of_range_behavior) parameter
determines how $s$ values outside the tabulated coordinate range are handled:

- `error` (default) reports an error;
- `clamp` returns the value at the nearest endpoint; and
- `extrapolate` linearly extrapolates using the nearest two profile stations.

## Example

The following TorchScript model maps the two inputs `[a, b]` to two profiles at coordinates
`[0, 1, 2]` and can be saved to `two_profiles.pt` for use with this material:

```python
import torch
from torch import nn


class TwoProfileModel(nn.Module):
    def forward(self, x):
        # x is expected to have shape [1, 2] containing [a, b]
        a = x[0, 0]
        b = x[0, 1]

        stations = torch.tensor([0.0, 1.0, 2.0], dtype=x.dtype, device=x.device)

        # Given inputs [a, b], produce
        # [[a, a + 2, a + 4],
        #  [b, b - 1, b - 2]].
        profile_a = a + 2.0 * stations
        profile_b = b - 1.0 * stations

        return torch.stack((profile_a, profile_b), dim=0)


model = TwoProfileModel()
scripted_model = torch.jit.script(model)
scripted_model.save("two_profiles.pt")
```

For the input `[10, 5]`, this model returns

!equation
\begin{bmatrix}
10 & 12 & 14 \\
5 & 4 & 3
\end{bmatrix}.

The first row defines `profile_a`, and the second row defines `profile_b`.

The material is configured in the test input as follows:

!listing modules/navier_stokes/test/tests/functormaterials/torchscript_profile_functor_material/profile.i
         block=UserObjects

!listing modules/navier_stokes/test/tests/functormaterials/torchscript_profile_functor_material/profile.i
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

!syntax parameters /FunctorMaterials/TorchScript1DProfileFunctorMaterial

!syntax inputs /FunctorMaterials/TorchScript1DProfileFunctorMaterial

!syntax children /FunctorMaterials/TorchScript1DProfileFunctorMaterial

!if-end!

!else
This object evaluates a TorchScript model that returns sampled one-dimensional profiles and
publishes those profiles as interpolated functor material properties.

!include libtorch/libtorch_warning.md
