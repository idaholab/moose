# VariationalDerivativeAction

!syntax description /VariationalProblem/VariationalDerivativeAction

## Overview

The `VariationalDerivativeAction` is a powerful action system that automatically generates weak form kernels from either energy functionals or strong form equations. It supports both variational formulations (where you provide an energy functional) and strong form PDEs (where you provide the equations directly).

## Features

### Energy Functional Approach

For variational problems, you can specify an energy functional and the action will automatically compute the variational derivatives to generate the weak form:

```
F[u] = ∫ f(u, ∇u, ∇²u, ...) dx
```

The variational derivative (Euler-Lagrange equation) is:

```
δF/δu = ∂f/∂u - ∇·(∂f/∂∇u) + ∇²:(∂f/∂∇²u) - ...
```

### Strong Form Approach

Alternatively, you can directly specify strong form PDEs using mathematical notation:

- Time-dependent equations: `c_t = expression` or `dt(c) = expression`
- Steady-state equations: `var = expression` (interpreted as `0 = expression - var`)

The action automatically:
1. Separates time derivative terms for MOOSE's `TimeDerivative` kernel
2. Derives the weak form through integration by parts
3. Generates appropriate kernels for each equation

### Expression Parsing

The action includes a comprehensive mathematical expression parser supporting:

- **Basic operations**: `+`, `-`, `*`, `/`, `^`
- **Differential operators**: `grad()`, `div()`, `laplacian()`, `curl()`
- **Vector operations**: `vec()`, `dot()`, `cross()`, `norm()`, `normalize()`
- **Tensor operations**: `trace()`, `det()`, `inv()`, `transpose()`, `sym()`, `contract()`
- **Mathematical functions**: `exp()`, `log()`, `sin()`, `cos()`, `sqrt()`, `abs()`

### Variable Splitting

For problems with higher-order derivatives beyond what finite elements support, the action can automatically introduce auxiliary variables. For example, a 4th-order Cahn-Hilliard equation can be split into two 2nd-order equations.

## Syntax

!syntax parameters /VariationalProblem/VariationalDerivativeAction

## Input Parameters

!syntax list /VariationalProblem/VariationalDerivativeAction objects=True actions=False subsystems=False

!syntax list /VariationalProblem/VariationalDerivativeAction objects=False actions=False subsystems=True

!syntax list /VariationalProblem/VariationalDerivativeAction objects=False actions=True subsystems=False

## Example 1: Cahn-Hilliard Energy Functional

Using the energy functional approach:

```
[VariationalProblem]
  [cahn_hilliard]
    type = VariationalDerivativeAction
    energy_type = expression
    
    # Energy functional: F[c] = ∫ [W(c) + κ/2|∇c|²] dx
    energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
    
    parameters = 'kappa=0.01'
    variables = 'c'
    
    use_automatic_differentiation = true
  []
[]
```

## Example 2: Strong Form PDEs

Using strong form equations with automatic weak form derivation:

```
[VariationalProblem]
  [coupled_system]
    type = VariationalDerivativeAction
    energy_type = expression
    
    # Intermediate expressions (MOOSE splits on semicolons)
    expressions = 'M = 1.0'
                  'dW_dc = 4*c*(c^2 - 1)'
    
    # Strong form equations
    strong_forms = 'c_t = div(M*grad(mu))'
                   'mu = dW_dc - kappa*laplacian(c)'
    
    parameters = 'kappa=0.01'
    variables = 'c mu'
  []
[]
```

## Example 3: Coupled Phase-Field Mechanics

A complex multi-physics example with vector assembly:

```
[VariationalProblem]
  [mechanics]
    type = VariationalDerivativeAction
    energy_type = expression
    
    # Build displacement vector from components
    expressions = 'u = vec(disp_x, disp_y)'
                  'strain = sym(grad(u))'
                  'stress = lambda*trace(strain)*I + 2*mu*strain'
    
    # Coupled energy functional
    energy_expressions = 'c = W(c) + 0.5*kappa*dot(grad(c), grad(c)) + alpha*c*trace(strain)'
                          'disp_x = 0.5*lambda*pow(trace(strain), 2) + mu*contract(strain, strain)'
                          'disp_y = 0.5*lambda*pow(trace(strain), 2) + mu*contract(strain, strain)'
    
    parameters = 'kappa=0.01 lambda=100 mu=75 alpha=10'
    variables = 'c disp_x disp_y'
  []
[]
```

## Generated Objects

The action automatically creates:

1. **Variables**: Primary field variables and auxiliary split variables if needed
2. **Kernels**: 
   - `ExpressionEvaluationKernel` for weak form residuals
   - `TimeDerivative` kernels for transient terms
3. **AuxKernels**: For auxiliary variable updates in split formulations
4. **Boundary Conditions**: Natural boundary conditions when specified

## Implementation Details

### Weak Form Generation

For an energy functional `F[u]`, the weak form residual is:

```
R = ∫ (∂f/∂u·v - ∂f/∂∇u·∇v + ∂f/∂∇²u:∇²v - ...) dx
```

where `v` is the test function.

### Automatic Differentiation

When `use_automatic_differentiation = true`, the action computes Jacobian terms symbolically or uses AD at runtime, eliminating the need for manual Jacobian derivation.

### Performance Considerations

- Expression evaluation uses optimized tape-based evaluation when possible
- Common subexpressions are detected and computed once
- Symbolic simplification reduces computational cost

## See Also

- [ExpressionEvaluationKernel.md] - The kernel that evaluates the generated expressions
- [VariationalKernelBase.md] - Base class for variational kernels

!syntax children /VariationalProblem/VariationalDerivativeAction