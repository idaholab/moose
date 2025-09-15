# VectorGradientConstraint

!syntax description /Kernels/VectorGradientConstraint

## Overview

`VectorGradientConstraint` enforces gradient constraints for vector auxiliary variables in variable splitting schemes. It is used when the auxiliary variable represents a vector quantity (like the full gradient of a scalar field) in the splitting of higher-order PDEs.

## Purpose

This kernel is essential for:
- Splitting scalar PDEs with vector gradient auxiliary variables
- Problems where the full gradient vector must be tracked
- Mixed formulations requiring gradient as an independent variable
- Fourth-order problems split into second-order systems

## Theory

### Vector Gradient Constraint

For a vector auxiliary variable **q** representing the gradient:

```
q = ∇u
```

where `u` is a scalar field and **q** is a vector field.

### Weak Form

The strong form constraint:
```
q - ∇u = 0
```

Has the weak form (component-wise):
```
∫_Ω (q_i·v_i + u·∂v_i/∂x_i) dx = 0
```

where `v_i` is the test function for component `i` of **q**.

### Full System

For the complete vector constraint:
```
∫_Ω (q·v + u(∇·v)) dx = 0
```

## Mathematical Formulation

### Component Form

Each component of the vector constraint:

```
R_x = ∫_Ω (q_x·v_x + u·∂v_x/∂x) dx
R_y = ∫_Ω (q_y·v_y + u·∂v_y/∂y) dx
R_z = ∫_Ω (q_z·v_z + u·∂v_z/∂z) dx
```

### Tensor Form

For higher-order constraints (e.g., **q** = ∇²u):

```
R_ij = ∫_Ω (q_ij·v_ij - ∂u/∂x_i · ∂v_ij/∂x_j) dx
```

## Implementation

```cpp
Real VectorGradientConstraint::computeQpResidual()
{
  // Constraint: q = ∇u
  // This kernel acts on one component of q

  Real residual = _u[_qp] * _test[_i][_qp];

  // Add divergence term for this component
  residual += _coupled_var[_qp] * _grad_test[_i][_qp](_component);

  return residual;
}
```

## Syntax

!syntax parameters /Kernels/VectorGradientConstraint

## Input Parameters

### Required Parameters

- `variable`: Component of the vector auxiliary variable
- `coupled_variable`: The scalar primary variable
- `component`: Which component this kernel handles (0=x, 1=y, 2=z)

### Optional Parameters

- `use_automatic_differentiation`: Enable AD for Jacobian
- `constraint_type`: Type of constraint (gradient, hessian, etc.)

## Examples

### Example 1: Fourth-Order Cahn-Hilliard with Vector Gradient

```
[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
  []
  [grad_c_x]  # x-component of ∇c
    order = FIRST
    family = LAGRANGE
  []
  [grad_c_y]  # y-component of ∇c
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  # Constraint: grad_c = ∇c
  [gradient_x]
    type = VectorGradientConstraint
    variable = grad_c_x
    coupled_variable = c
    component = 0
  []

  [gradient_y]
    type = VectorGradientConstraint
    variable = grad_c_y
    coupled_variable = c
    component = 1
  []

  # Use grad_c in the main equation
  [cahn_hilliard]
    type = VariationalKernelBase
    variable = c
    energy_expression = 'W(c) + 0.5*lambda*(grad_c_x^2 + grad_c_y^2)'
  []
[]
```

### Example 2: Biharmonic with Full Gradient Tracking

```
[Variables]
  [u]
  []
  [grad_u_x]
  []
  [grad_u_y]
  []
  [laplacian_u]
  []
[]

[Kernels]
  # First constraint: grad_u = ∇u
  [grad_constraint_x]
    type = VectorGradientConstraint
    variable = grad_u_x
    coupled_variable = u
    component = 0
  []

  [grad_constraint_y]
    type = VectorGradientConstraint
    variable = grad_u_y
    coupled_variable = u
    component = 1
  []

  # Second constraint: laplacian_u = ∇·grad_u
  [laplacian_constraint]
    type = Reaction
    variable = laplacian_u
    # Computed from divergence of grad_u
  []

  # Main equation using split variables
  [biharmonic]
    type = Diffusion
    variable = laplacian_u
  []
[]
```

### Example 3: Surface Diffusion with Gradient

For surface diffusion ∂c/∂t = ∇·(M∇μ) where μ = -∇²c:

```
[Variables]
  [c]
  []
  [mu]
  []
  [grad_mu_x]
  []
  [grad_mu_y]
  []
[]

[Kernels]
  # Time derivative
  [c_dot]
    type = TimeDerivative
    variable = c
  []

  # Constraint: grad_mu = ∇μ
  [grad_mu_x_constraint]
    type = VectorGradientConstraint
    variable = grad_mu_x
    coupled_variable = mu
    component = 0
  []

  [grad_mu_y_constraint]
    type = VectorGradientConstraint
    variable = grad_mu_y
    coupled_variable = mu
    component = 1
  []

  # Flux divergence using split gradient
  [flux_divergence]
    type = ConservativeAdvection
    variable = c
    velocity_x = grad_mu_x
    velocity_y = grad_mu_y
  []

  # Chemical potential
  [chemical_potential]
    type = Diffusion
    variable = mu
  []
[]
```

## Automatic Generation

The `AutomaticWeakFormAction` creates these kernels when:

1. A scalar variable has gradient terms beyond FE capability
2. Vector representation of gradient is needed
3. The split creates vector auxiliary variables

Example automatic generation:

```
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = 'W(c) + lambda*|∇²c|²'
  variables = 'c'
  enable_splitting = true
  max_fe_order = 1
[]

# Automatically generates:
# - Variables: c, c_grad_x, c_grad_y
# - VectorGradientConstraint kernels for each component
```

## Numerical Considerations

### System Structure

Vector constraints create a saddle-point system:

```
[K  B^T] [u]   [f]
[B  0  ] [q] = [0]
```

where:
- K: Stiffness matrix for primary equation
- B: Constraint matrix
- u: Primary variable
- q: Vector auxiliary variable

### Preconditioning

Effective preconditioners for saddle-point systems:

```
[Preconditioning]
  [fsp]
    type = FSP
    # Fieldsplit preconditioner for saddle-point
    topsplit = 'u q'
    [u]
      vars = 'c'
      petsc_options_iname = '-pc_type'
      petsc_options_value = 'hypre'
    []
    [q]
      vars = 'grad_c_x grad_c_y'
      petsc_options_iname = '-pc_type'
      petsc_options_value = 'jacobi'
    []
  []
[]
```

### Stability

- Inf-sup condition must be satisfied
- Use compatible discretizations
- May need stabilization for equal-order interpolations

## Performance

### Memory Considerations

Vector splitting increases DOFs:
- Original: N DOFs for scalar field
- Split: N + d×N DOFs (d = dimension)

### Optimization Strategies

1. **Block solvers**: Exploit structure
2. **Multigrid**: For each field separately
3. **Schur complement**: Eliminate auxiliary variables

## Verification

Check constraint satisfaction:

```
[Postprocessors]
  [grad_error_x]
    type = L2Difference
    variable = grad_c_x
    other_variable = actual_grad_c_x  # Computed directly
  []

  [grad_error_y]
    type = L2Difference
    variable = grad_c_y
    other_variable = actual_grad_c_y
  []
[]
```

## Common Issues

### Issue: Locking or Spurious Modes

**Solution**: Use appropriate element pairs (e.g., Taylor-Hood)

### Issue: Poor Convergence

**Solution**:
- Check compatibility of discretizations
- Use appropriate preconditioner
- Scale variables properly

### Issue: Constraint Violation

**Solution**:
- Tighten solver tolerances
- Check boundary conditions
- Verify weak form implementation

## See Also

- [GradientConstraintKernel.md] - General gradient constraints
- [ScalarGradientConstraint.md] - Scalar gradient constraints
- [AutomaticWeakFormAction.md] - Automatic constraint generation
- [VariationalKernelBase.md] - Primary equation kernels

!syntax children /Kernels/VectorGradientConstraint