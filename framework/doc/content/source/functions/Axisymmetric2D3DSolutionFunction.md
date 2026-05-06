# Axisymmetric2D3DSolutionFunction

!syntax description /Functions/Axisymmetric2D3DSolutionFunction

The 2D solution is likely to be the output of a 2D-RZ calculation, which we want to compare
to a full 3D model. This is useful for assessing the validity of the 2D-RZ geometric approximation.

The axis of symmetry for the original 2D axisymmetric calculation and for mapping
this 2D axisymmetric calculation into the 3D space can both be specified.

## Scalar and vector modes

In this example, three `Axisymmetric2D3DSolutionFunction` objects are used to load results for a 2D
axisymmetric simulation in a 3D mechanics simulation. These results are then used in the `BCs`
block to impose a displacement on a boundary, and in a temperature variable using a [FunctionAux.md].

!listing modules/combined/test/tests/axisymmetric_2d3d_solution_function/3dy.i block=Functions

## Tensor mode

The function can also map a rank-two tensor field saved in 2D axisymmetric (cylindrical) components
into a 3D Cartesian tensor. This is useful when re-imposing a stored deformation-dependent field
(such as plastic eigenstrain) on a 3D mesh after an axisymmetric analysis.

### Input specification

Tensor mode is selected by providing 4 variable names in [!param](/Functions/Axisymmetric2D3DSolutionFunction/from_variables).
These four names correspond to the cylindrical tensor components in the order: `rr`, `yy`, `ry`, `tt`.
A typical usage pattern is to read components saved by [RankTwoTensorAux](https://mooseframework.inl.gov/)
with suffixes `_00`, `_11`, `_01`, `_22` respectively (the naming convention for the radial-radial,
axial-axial, radial-axial shear, and hoop-hoop components of a rank-two tensor stored in cylindrical coordinates).

Each unique Cartesian component `(i, j)` is extracted by instantiating one `Axisymmetric2D3DSolutionFunction`
with [!param](/Functions/Axisymmetric2D3DSolutionFunction/component_i) and [!param](/Functions/Axisymmetric2D3DSolutionFunction/component_j)
set to the desired row and column (0-2). The six unique symmetric Cartesian components are then assembled
into a rank-two tensor property (e.g., by [GenericFunctionRankTwoTensor](https://mooseframework.inl.gov/)).

### Example: assembling a tensor property

The following pattern instantiates six functions to extract the six unique Cartesian components,
which are then assembled into a rank-two tensor property using `GenericFunctionRankTwoTensor`:

```
[Functions]
  [soln_xx]
    type = Axisymmetric2D3DSolutionFunction
    solution = solution_uo
    from_variables = 'eigenstrain_rr eigenstrain_yy eigenstrain_ry eigenstrain_tt'
    component_i = 0
    component_j = 0
  []
  [soln_yy]
    type = Axisymmetric2D3DSolutionFunction
    solution = solution_uo
    from_variables = 'eigenstrain_rr eigenstrain_yy eigenstrain_ry eigenstrain_tt'
    component_i = 1
    component_j = 1
  []
  [soln_zz]
    type = Axisymmetric2D3DSolutionFunction
    solution = solution_uo
    from_variables = 'eigenstrain_rr eigenstrain_yy eigenstrain_ry eigenstrain_tt'
    component_i = 2
    component_j = 2
  []
  [soln_xy]
    type = Axisymmetric2D3DSolutionFunction
    solution = solution_uo
    from_variables = 'eigenstrain_rr eigenstrain_yy eigenstrain_ry eigenstrain_tt'
    component_i = 0
    component_j = 1
  []
  [soln_yz]
    type = Axisymmetric2D3DSolutionFunction
    solution = solution_uo
    from_variables = 'eigenstrain_rr eigenstrain_yy eigenstrain_ry eigenstrain_tt'
    component_i = 1
    component_j = 2
  []
  [soln_xz]
    type = Axisymmetric2D3DSolutionFunction
    solution = solution_uo
    from_variables = 'eigenstrain_rr eigenstrain_yy eigenstrain_ry eigenstrain_tt'
    component_i = 0
    component_j = 2
  []
[]

[Materials]
  [eigenstrain_from_function]
    type = GenericFunctionRankTwoTensor
    tensor_name = eigenstrain
    tensor_functions = 'soln_xx soln_yy soln_zz soln_xy soln_yz soln_xz'
  []
[]
```

### On-axis behavior

At points where the radial distance `r = 0` (on the axis of symmetry), the cylindrical-frame rotation
is indeterminate. The function requires that the input tensor satisfy:

- `T_ry = 0` (no radial-axial shear on axis)
- `T_rr = T_tt` (radial and hoop components must be equal on axis)

If these conditions are not met, the function calls `mooseError` with one of:
- `In Axisymmetric2D3DSolutionFunction r=0 and T_ry != 0`
- `In Axisymmetric2D3DSolutionFunction r=0 and T_rr != T_tt`

When these conditions *are* satisfied, the on-axis tensor reduces to the diagonal form
`diag(T_rr, T_yy, T_rr)` in any Cartesian frame, ensuring a unique and continuous solution.

### Cross-linking

For a complete 2D-to-3D eigenstrain restart workflow, see the tutorial
[Eigenstrain Restart from 2D Axisymmetric to 3D](modules/solid_mechanics/examples/eigenstrain_restart_2d_to_3d.md optional=True).

!syntax parameters /Functions/Axisymmetric2D3DSolutionFunction

!syntax inputs /Functions/Axisymmetric2D3DSolutionFunction

!syntax children /Functions/Axisymmetric2D3DSolutionFunction
