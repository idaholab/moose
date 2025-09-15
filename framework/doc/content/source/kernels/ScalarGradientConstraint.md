# ScalarGradientConstraint

!syntax description /Kernels/ScalarGradientConstraint

## Overview

`ScalarGradientConstraint` is a specialized constraint kernel for scalar gradient relationships in variable splitting schemes. It enforces the constraint that an auxiliary scalar variable equals a component of the gradient of a primary variable.

## Purpose

This kernel is used when splitting higher-order PDEs where the auxiliary variable represents a scalar quantity derived from a gradient. Common in:
- 1D problems where gradients are naturally scalar
- Directional derivatives in multi-dimensional problems
- Splitting schemes for scalar PDEs with high-order derivatives

## Theory

### Scalar Gradient Constraint

For a scalar auxiliary variable `q` representing a gradient component:

```
q = ∂u/∂x  (in 1D)
q = n·∇u   (directional derivative)
```

### Weak Form

The strong form constraint:
```
q - ∂u/∂x = 0
```

Has the weak form (after integration by parts):
```
∫_Ω (q·v + u·∂v/∂x) dx = 0
```

where `v` is the test function.

## Mathematical Formulation

### 1D Problems

In one dimension, gradients are naturally scalar:

```
Residual = q·test + u·dtest/dx
```

### Multi-dimensional Problems

For a specific gradient component:

```
Residual = q·test + u·∇test·e_i
```

where `e_i` is the unit vector in the i-th direction.

## Implementation

```cpp
Real ScalarGradientConstraint::computeQpResidual()
{
  // Constraint: q = ∂u/∂x (or specified component)

  Real residual = _u[_qp] * _test[_i][_qp];

  if (_mesh.dimension() == 1)
  {
    // 1D: gradient is scalar
    residual += _coupled_var[_qp] * _grad_test[_i][_qp](0);
  }
  else
  {
    // Multi-D: specific component
    residual += _coupled_var[_qp] * _grad_test[_i][_qp](_component);
  }

  return residual;
}
```

## Syntax

!syntax parameters /Kernels/ScalarGradientConstraint

## Input Parameters

### Required Parameters

- `variable`: The scalar auxiliary variable
- `coupled_variable`: The primary variable whose gradient is constrained
- `component`: Which component of the gradient (0=x, 1=y, 2=z)

### Optional Parameters

- `use_automatic_differentiation`: Enable AD for Jacobian
- `scaling_factor`: Scale the constraint equation

## Examples

### Example 1: 1D Fourth-Order Problem

```
[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [u_xx]  # Second derivative in 1D
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  # Constraint: u_xx = d²u/dx²
  [second_deriv_constraint]
    type = ScalarGradientConstraint
    variable = u_xx
    coupled_variable = u_x  # Intermediate variable
    component = 0
  []
[]
```

### Example 2: Directional Derivative

For a directional derivative in 2D:

```
[Variables]
  [phi]
    order = FIRST
    family = LAGRANGE
  []
  [dphi_dn]  # Normal derivative
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  # Constraint: dphi_dn = n·∇φ
  [normal_deriv_constraint]
    type = ScalarGradientConstraint
    variable = dphi_dn
    coupled_variable = phi
    direction_vector = '0.707 0.707 0'  # 45° angle
  []
[]
```

### Example 3: Biharmonic in 1D

Solving ∇⁴u = f in 1D:

```
[Variables]
  [u]
  []
  [u_x]  # First derivative
  []
  [u_xx]  # Second derivative
  []
  [u_xxx]  # Third derivative
  []
[]

[Kernels]
  # Chain of constraints
  [first_deriv]
    type = ScalarGradientConstraint
    variable = u_x
    coupled_variable = u
  []

  [second_deriv]
    type = ScalarGradientConstraint
    variable = u_xx
    coupled_variable = u_x
  []

  [third_deriv]
    type = ScalarGradientConstraint
    variable = u_xxx
    coupled_variable = u_xx
  []

  # Main equation: d(u_xxx)/dx = f
  [biharmonic]
    type = Diffusion
    variable = u_xxx
  []
[]
```

## Relationship to Other Constraints

### Comparison with GradientConstraintKernel

- `ScalarGradientConstraint`: For scalar auxiliary variables
- `GradientConstraintKernel`: For general tensor auxiliary variables
- `VectorGradientConstraint`: For vector auxiliary variables

### When to Use

Use `ScalarGradientConstraint` when:
- Working in 1D (gradients are naturally scalar)
- Need a specific directional derivative
- Auxiliary variable represents a scalar quantity

## Numerical Considerations

### Well-posedness

- Constraint adds algebraic equation to system
- Results in saddle-point structure
- May require specialized solvers

### Preconditioning

For systems with constraints:

```
[Preconditioning]
  [smp]
    type = SMP
    full = true
    # Include constraint equations in preconditioner
  []
[]
```

### Scaling

Proper scaling improves conditioning:

```
[Kernels]
  [scaled_constraint]
    type = ScalarGradientConstraint
    variable = q
    coupled_variable = u
    scaling_factor = 1.0e-3  # Scale if variables have different magnitudes
  []
[]
```

## Verification

Verify constraint satisfaction:

```
[AuxVariables]
  [constraint_error]
  []
[]

[AuxKernels]
  [compute_error]
    type = ParsedAux
    variable = constraint_error
    expression = 'u_x - grad_u_x'
    coupled_variables = 'u_x u'
  []
[]

[Postprocessors]
  [max_constraint_error]
    type = NodalExtremeValue
    variable = constraint_error
    value_type = max
  []
[]
```

## Common Issues

### Issue: Poor Convergence

**Solution**: Check scaling between primary and auxiliary variables

### Issue: Oscillations in Solution

**Solution**: May need stabilization or different discretization

### Issue: Singular Matrix

**Solution**: Ensure proper boundary conditions for both primary and auxiliary variables

## See Also

- [GradientConstraintKernel.md] - General gradient constraints
- [VectorGradientConstraint.md] - Vector gradient constraints
- [AutomaticWeakFormAction.md] - Automatic generation of constraints
- [VariationalKernelBase.md] - Primary variational kernels

!syntax children /Kernels/ScalarGradientConstraint