# VariationalKernelBase

!syntax description /Kernels/VariationalKernelBase

## Overview

`VariationalKernelBase` is a base class for implementing kernels derived from variational principles. It provides a framework for computing weak form residuals and Jacobians from energy functionals using the calculus of variations.

This kernel is designed for problems where the governing equations arise from minimizing or finding stationary points of energy functionals.

## Theory

### Variational Formulation

For an energy functional:

```
F[u] = ∫_Ω f(x, u, ∇u, ∇²u, ...) dx
```

The variational derivative (Euler-Lagrange equation) is:

```
δF/δu = ∂f/∂u - ∇·(∂f/∂∇u) + ∇²:(∂f/∂∇²u) - ...
```

### Weak Form

The weak form is obtained by multiplying by a test function `v` and integrating:

```
R = ∫_Ω [∂f/∂u·v - ∂f/∂∇u·∇v + ∂f/∂∇²u:∇²v - ...] dx
```

Integration by parts moves derivatives from the solution to the test function, reducing continuity requirements.

## Features

### Energy Types

The kernel supports various predefined energy functionals:

- **Double-Well**: Phase-field models with `f = (u²-1)²`
- **Elastic**: Linear elasticity with strain energy
- **Neo-Hookean**: Nonlinear elasticity for large deformations
- **Surface Energy**: Interface/surface tension models
- **Cahn-Hilliard**: Phase separation dynamics
- **Custom**: User-defined energy expressions

### Automatic Differentiation

The kernel can compute Jacobians through:
1. Symbolic differentiation of the energy functional
2. Automatic differentiation using dual numbers
3. Numerical finite differences (fallback)

### Variable Splitting

For higher-order derivatives, automatic variable splitting converts:
- 4th-order → Two 2nd-order equations
- 6th-order → Three 2nd-order equations

This enables standard finite element discretization.

## Syntax

!syntax parameters /Kernels/VariationalKernelBase

## Input Parameters

!syntax list /Kernels/VariationalKernelBase objects=True actions=False subsystems=False

!syntax list /Kernels/VariationalKernelBase objects=False actions=False subsystems=True

!syntax list /Kernels/VariationalKernelBase objects=False actions=True subsystems=False

## Example 1: Allen-Cahn Equation

```
[Kernels]
  [allen_cahn]
    type = VariationalKernelBase
    variable = eta
    energy_type = double_well
    
    # Energy: F = ∫ [(η²-1)² + κ|∇η|²] dx
    gradient_coefficient = 0.5  # κ
    well_height = 1.0
    
    use_automatic_differentiation = true
  []
[]
```

## Example 2: Linear Elasticity

```
[Kernels]
  [elastic_x]
    type = VariationalKernelBase
    variable = disp_x
    energy_type = elastic_linear
    
    # Strain energy: W = λ/2(tr(ε))² + μ|ε|²
    elastic_lambda = 100
    elastic_mu = 75
    
    coupled_variables = 'disp_y disp_z'
  []
[]
```

## Example 3: Custom Energy Functional

```
[Kernels]
  [custom]
    type = VariationalKernelBase
    variable = c
    energy_type = custom
    
    # Custom energy with coupling
    energy_expression = 'c^4/4 - c^2/2 + kappa*dot(grad(c), grad(c)) + alpha*c*T'
    
    coupled_variables = 'T'  # Temperature coupling
    parameters = 'kappa=0.01 alpha=0.1'
    
    use_automatic_differentiation = true
  []
[]
```

## Energy Functional Specification

### Built-in Energy Types

#### Double-Well Potential
```
f = well_height * (u² - 1)²
```
Used in phase-field models for phase transitions.

#### Gradient Energy
```
f = gradient_coefficient * |∇u|²
```
Penalizes spatial variations, promoting smooth solutions.

#### Elastic Energy
```
f = λ/2 * (tr(ε))² + μ * tr(ε²)
ε = sym(∇u)  # Strain tensor
```
Linear elasticity for small deformations.

#### Cahn-Hilliard Energy
```
f = (c² - 1)² + κ|∇c|²
```
Phase separation with interfacial energy.

### Custom Energy Expressions

For complex energy functionals, use string expressions:

```
energy_expression = 'W(c) + 0.5*kappa*norm(grad(c))^2 + coupling_term'
```

Supported operations include all mathematical functions, differential operators, and tensor operations.

## Implementation Details

### Residual Computation

The residual at quadrature point `qp` is computed as:

```cpp
Real VariationalKernelBase::computeQpResidual()
{
  // Compute energy density partial derivatives
  Real df_du = computePartialDerivative(_u[_qp], 0);
  RealGradient df_dgrad = computePartialDerivative(_grad_u[_qp], 1);
  
  // Weak form: test * df/du - grad_test · df/d(grad_u)
  return _test[_i][_qp] * df_du - _grad_test[_i][_qp] * df_dgrad;
}
```

### Jacobian Computation

The Jacobian contribution is:

```cpp
Real VariationalKernelBase::computeQpJacobian()
{
  // Second derivatives of energy functional
  Real d2f_du2 = computeSecondDerivative(_u[_qp], 0, 0);
  Real d2f_du_dgrad = computeSecondDerivative(_u[_qp], _grad_u[_qp], 0, 1);
  
  // Jacobian terms
  return _test[_i][_qp] * d2f_du2 * _phi[_j][_qp]
         - _grad_test[_i][_qp] * d2f_du_dgrad * _grad_phi[_j][_qp];
}
```

### Performance Optimization

- **Caching**: Repeated computations cached within quadrature loops
- **Symbolic Simplification**: Expressions simplified at parse time
- **Vectorization**: SIMD operations for array computations
- **Sparsity**: Zero terms detected and skipped

## Advanced Features

### Multi-Field Problems

For coupled field problems, derive from `VariationalKernelBase`:

```cpp
class CoupledVariationalKernel : public VariationalKernelBase
{
  // Override energy computation
  virtual Real computeEnergyDensity() override
  {
    return _c[_qp] * _u[_qp] + 0.5 * _grad_u[_qp].norm_sq();
  }
};
```

### Stabilization

Add stabilization terms for advection-dominated problems:

```
[Kernels]
  [stabilized]
    type = VariationalKernelBase
    variable = u
    enable_stabilization = true
    stabilization_type = SUPG
    stabilization_parameter = 0.1
  []
[]
```

### Conservation Properties

The kernel can verify conservation laws:

```
validate_conservation = true  # Check mass/energy conservation
check_stability = true        # Verify energy decrease
```

## Troubleshooting

### Common Issues

1. **Non-convergence**: Check energy convexity, may need relaxation
2. **Oscillations**: Add gradient regularization term
3. **Slow convergence**: Enable automatic differentiation for exact Jacobian
4. **High condition number**: Consider variable scaling or preconditioning

### Debugging

Enable verbose output to debug energy evaluations:

```
output_energy_density = true
output_weak_form = true
weak_form_file = 'weak_form.txt'
```

## See Also

- [VariationalDerivativeAction.md] - Automatic kernel generation from energy
- [ExpressionEvaluationKernel.md] - Alternative expression-based kernel
- Phase-field module documentation

!syntax children /Kernels/VariationalKernelBase