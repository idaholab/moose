# ExpressionEvaluationKernel

!syntax description /Kernels/ExpressionEvaluationKernel

## Overview

The `ExpressionEvaluationKernel` is a generic kernel that evaluates arbitrary mathematical expressions to compute residuals and Jacobians. It is primarily used by the `VariationalDerivativeAction` to implement weak forms derived from energy functionals or strong form equations.

This kernel provides runtime evaluation of symbolic expressions, eliminating the need to write custom C++ kernels for each PDE.

## Features

### Expression Evaluation

The kernel evaluates mathematical expressions at quadrature points, supporting:

- Field variables and their derivatives (gradients, Hessians)
- Test functions and shape functions
- Coupled variables
- Parameters and constants
- Complex mathematical operations

### Automatic Differentiation

When enabled, the kernel can compute Jacobian terms automatically using:
- Symbolic differentiation of the residual expression
- Tape-based automatic differentiation
- Finite difference approximation as fallback

### Coupled Systems

Full support for multi-physics problems through coupled variables:
- Access to coupled field values and gradients
- Off-diagonal Jacobian computation
- Consistent linearization for Newton convergence

## Syntax

!syntax parameters /Kernels/ExpressionEvaluationKernel

## Input Parameters

!syntax list /Kernels/ExpressionEvaluationKernel objects=True actions=False subsystems=False

!syntax list /Kernels/ExpressionEvaluationKernel objects=False actions=False subsystems=True

!syntax list /Kernels/ExpressionEvaluationKernel objects=False actions=True subsystems=False

## Example 1: Simple Diffusion

```
[Kernels]
  [diffusion]
    type = ExpressionEvaluationKernel
    variable = u
    
    # Residual: ∇·(D∇u) = D∇²u (after integration by parts: -D∇u·∇v)
    residual_expression = '-D*dot(grad(u), grad(test))'
    
    parameters = 'D=1.0'
    
    use_automatic_differentiation = true
  []
[]
```

## Example 2: Nonlinear Reaction-Diffusion

```
[Kernels]
  [reaction_diffusion]
    type = ExpressionEvaluationKernel
    variable = c
    
    # Allen-Cahn equation residual
    residual_expression = 'test*(c^3 - c) - kappa*dot(grad(c), grad(test))'
    
    # Optional: provide explicit Jacobian
    jacobian_expression = 'test*(3*c^2 - 1)*phi - kappa*dot(grad(phi), grad(test))'
    
    parameters = 'kappa=0.01'
  []
[]
```

## Example 3: Coupled Multi-Physics

```
[Kernels]
  [coupled_term]
    type = ExpressionEvaluationKernel
    variable = u
    coupled_variables = 'v w'
    
    # Residual with coupling
    residual_expression = 'test*u*v - alpha*dot(grad(w), grad(test))'
    
    # Off-diagonal Jacobian for variable v
    off_diagonal_jacobian_expressions = 'v = test*u*phi'
    
    parameters = 'alpha=1.5'
    
    use_automatic_differentiation = true
  []
[]
```

## Expression Syntax

### Available Functions

The kernel supports a rich set of mathematical functions:

#### Differential Operators
- `grad(u)` - Gradient of field u
- `div(v)` - Divergence of vector field v
- `laplacian(u)` - Laplacian of field u
- `curl(v)` - Curl of vector field (3D only)

#### Vector Operations
- `dot(a, b)` - Dot product
- `cross(a, b)` - Cross product (3D only)
- `norm(v)` - Euclidean norm
- `normalize(v)` - Unit vector

#### Tensor Operations
- `trace(A)` - Trace of tensor
- `det(A)` - Determinant
- `inv(A)` - Inverse
- `transpose(A)` - Transpose
- `sym(A)` - Symmetric part: (A + A^T)/2
- `contract(A, B)` - Double contraction: A:B

#### Mathematical Functions
- `exp()`, `log()`, `sqrt()`, `abs()`
- `sin()`, `cos()`, `tan()`
- `pow(base, exponent)`

### Special Variables

- `test` - Test function value at quadrature point
- `grad(test)` - Test function gradient
- `phi` - Shape function (for Jacobian)
- `grad(phi)` - Shape function gradient
- `x`, `y`, `z` - Spatial coordinates
- `t` - Time

## Implementation Details

### Evaluation Context

At each quadrature point, the kernel sets up an evaluation context containing:

1. **Field Values**: Current solution values and gradients
2. **Test Functions**: Test function and gradient for residual
3. **Shape Functions**: Shape function and gradient for Jacobian
4. **Parameters**: User-defined constants
5. **Position**: Current quadrature point location
6. **Time**: Current simulation time

### Performance Optimization

The kernel employs several optimization strategies:

- **Expression Compilation**: Symbolic expressions are parsed once and stored as AST
- **Tape-Based Evaluation**: Repeated evaluations use optimized tape operations
- **Common Subexpression Elimination**: Identical subexpressions computed once
- **Vectorization**: SIMD operations where applicable

### Jacobian Computation

The Jacobian can be computed through:

1. **Explicit Expression**: User provides `jacobian_expression`
2. **Automatic Differentiation**: Symbolic or tape-based AD of residual
3. **Finite Differences**: Numerical approximation (fallback)

Priority: Explicit > AD > Finite Differences

## Transient Problems

For time-dependent problems, the kernel can work with MOOSE's time integration:

```
[Kernels]
  # Time derivative handled by MOOSE
  [time]
    type = TimeDerivative
    variable = u
  []
  
  # Spatial terms
  [spatial]
    type = ExpressionEvaluationKernel
    variable = u
    residual_expression = '...'
    is_transient_term = false  # This kernel handles spatial terms only
  []
[]
```

## Error Messages

Common error messages and solutions:

- **"Undefined variable: X"**: Variable X not declared in `coupled_variables`
- **"Type mismatch in operation"**: Check tensor/vector dimensions match
- **"Division by zero"**: Add guards for denominators (e.g., `norm(v) + eps`)
- **"Function not found: X"**: Function X not supported, check spelling

## See Also

- [VariationalDerivativeAction.md] - Action that generates these kernels
- [VariationalKernelBase.md] - Alternative kernel base class

!syntax children /Kernels/ExpressionEvaluationKernel