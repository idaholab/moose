# AutomaticWeakFormAction

!syntax description /AutomaticWeakForm/AutomaticWeakFormAction

## Overview

The `AutomaticWeakFormAction` is a powerful action system that automatically generates weak form kernels from energy functionals. It computes variational derivatives symbolically to create the appropriate residual and Jacobian contributions for finite element analysis.

## Features

### Energy Functional Approach

For variational problems, you specify an energy functional and the action automatically computes the variational derivatives to generate the weak form:

```
F[u] = ∫ f(u, ∇u, ∇²u, ...) dx
```

The variational derivative (Euler-Lagrange equation) is:

```
δF/δu = ∂f/∂u - ∇·(∂f/∂∇u) + ∇²:(∂f/∂∇²u) - ...
```

### Expression Parsing

The action includes a comprehensive mathematical expression parser. Arithmetic operators (`+`, `-`, `*`, `/`, `^`) are handled alongside a catalogue of differential, vector, tensor, and transcendental helpers. Custom symbols can be registered through `StringExpressionParser::registerFunction`. See [Supported Operators and Functions](#supported-operators-and-functions) for the complete reference.

### Variable Splitting for Higher-Order PDEs

For problems with derivatives beyond the supported finite-element order, the action can introduce additional primary variables (not auxiliary variables) together with constraint energies to enforce the derivative relationships. For example, a 4th-order Cahn–Hilliard energy containing `∇⁴c` yields `q = ∇²c` and transforms the energy into `λ|q|² + ...`. The resulting system stays fully coupled in Newton solves.

### Coupled Multi-Physics Problems

The action supports coupled multi-physics problems where multiple variables interact through the energy functional. Variables are automatically coupled in the generated kernels.

## Syntax

!syntax parameters /AutomaticWeakForm/AutomaticWeakFormAction

## Input Parameters

### Required Parameters

- `energy_type`: Type of energy functional (EXPRESSION, DOUBLE_WELL, ELASTIC, etc.)
- `variables`: List of field variables in the energy functional

### Expression Parameters

- `energy_expression`: Mathematical expression for the energy density
- `parameters`: Named parameters as key-value pairs (e.g., 'kappa 0.01 lambda 100')

### Variable Splitting Parameters

- `enable_splitting`: Enable automatic variable splitting for higher-order derivatives
- `max_fe_order`: Maximum finite element order available (default: 2)

### Coupling Parameters

- `coupled_variables`: Additional variables to couple (automatically determined from energy expression)

### Numerical Parameters

- `use_automatic_differentiation`: Use AD for Jacobian computation (default: true)
- `compute_jacobian_numerically`: Use finite differences for Jacobian (default: false)
- `fd_eps`: Finite difference epsilon (default: 1e-8)

### Output Control

- `verbose`: Enable detailed output for debugging

## Example 1: Cahn-Hilliard Energy Functional

Simple Cahn-Hilliard equation:

```
[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Energy functional: F[c] = ∫ [W(c) + κ/2|∇c|²] dx
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'

  parameters = 'kappa 0.01'
  variables = 'c'

  use_automatic_differentiation = true
[]
```

## Example 2: Fourth-Order Cahn-Hilliard with Splitting

Cahn-Hilliard with 4th-order regularization:

```
[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Energy with 4th-order term requiring splitting
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*dot(grad(grad(c)), grad(grad(c)))'

  parameters = 'kappa 0.01 lambda 0.001'
  variables = 'c'

  # Enable automatic splitting for 4th-order term
  enable_splitting = true
  max_fe_order = 1  # Forces splitting since we need 2nd derivatives

  use_automatic_differentiation = true
[]
```

## Example 3: Coupled Phase-Field and Thermal Diffusion

Multi-physics coupling between concentration and temperature:

```
[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Coupled energy functional
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*k_th*dot(grad(T), grad(T)) + beta*c*T'

  parameters = 'kappa 0.01 k_th 1.0 beta 0.1'
  variables = 'c T'

  use_automatic_differentiation = true
[]
```

## Example 4: Phase-Field with Mechanics

Coupled Cahn-Hilliard and elasticity (simplified):

```
[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Chemical + elastic energy (simplified without proper tensor mechanics)
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*(dot(grad(disp_x), grad(disp_x)) + dot(grad(disp_y), grad(disp_y))) + alpha*c*(disp_x + disp_y)'

  parameters = 'kappa 0.01 lambda 100 mu 75 alpha 10'
  variables = 'c disp_x disp_y'

  use_automatic_differentiation = true
[]
```

## Generated Objects

The action automatically creates:

1. **Variables**:
   - Primary field variables specified in `variables`
   - Split `MooseVariable`s for higher-order derivatives (e.g., `c_grad`, `c_hess`)

2. **Kernels**:
   - `VariationalKernelBase` for each primary or split variable
   - Constraint kernels derived from the generated split-variable energies
   - Kernels automatically handle all required couplings

3. **Coupling**:
   - Variables are automatically coupled based on the energy expression
   - Each kernel receives the appropriate coupled variables

## Implementation Details

### Weak Form Generation

For an energy functional `F[u]`, the weak form residual is computed as:

```
R = ∫ (C⁰·v - C¹·∇v + C²:∇²v - ...) dx
```

where:
- `C^k = ∂f/∂(∇^k u)` are the variational derivative coefficients
- `v` is the test function

### Automatic Differentiation

When `use_automatic_differentiation = true`:
- Symbolic differentiation computes exact derivatives
- AD provides automatic Jacobian computation
- No manual derivative implementation required

### Variable Splitting Algorithm

1. Analyzes energy expression for maximum derivative order
2. If order > `max_fe_order`, introduces split variables:
   - 1st order split: `u_grad = ∇u`
   - 2nd order split: `u_hess = ∇²u`
3. Transforms energy expression using split variables
4. Generates constraint kernels to enforce the relationships

### Performance Considerations

- Expression evaluation is optimized through symbolic simplification
- Common subexpressions are detected and computed once
- AD provides efficient Jacobian computation
- Variable splitting enables solving high-order PDEs with standard elements

## Debugging

Enable `verbose = true` to see:
- Parsed energy expression
- Generated split variables
- Transformed expressions
- Created kernels and couplings

## See Also

- [VariationalKernelBase.md] - Base class for variational kernels
- [GradientConstraintKernel.md] - Enforces gradient constraints for split variables
- [ScalarGradientConstraint.md] - Scalar gradient constraint kernel
- [VectorGradientConstraint.md] - Vector gradient constraint kernel

!syntax children /AutomaticWeakForm/AutomaticWeakFormAction

## Supported Operators and Functions

| Category | Operators / Functions | Notes |
| --- | --- | --- |
| Arithmetic | `+`, `-`, unary `-`, `*`, `/`, `^`, `pow(a, b)` | Standard scalar, vector, and tensor arithmetics obey broadcasting rules defined by the AST builders. |
| Differential | `grad(...)`, `gradient(...)`, `div(...)`, `divergence(...)`, `laplacian(...)`, `laplace(...)`, `hessian(...)`, `curl(...)` | `curl` is restricted to 3D; `hessian` is represented as a gradient of a gradient. |
| Vector & Tensor | `dot(...)`, `cross(...)`, `norm(...)`, `magnitude(...)`, `normalize(...)`, `trace(...)`, `tr(...)`, `det(...)`, `determinant(...)`, `inv(...)`, `inverse(...)`, `transpose(...)`, `trans(...)`, `sym(...)`, `symmetric(...)`, `skew(...)`, `dev(...)`, `deviatoric(...)`, `contract(...)`, `outer(...)`, `vec(...)`, `vector(...)` | `vec` assembles vectors from scalar components; contraction follows index-based shape rules. |
| Scalar Functions | `exp(...)`, `log(...)`, `ln(...)`, `sin(...)`, `cos(...)`, `tan(...)`, `sqrt(...)`, `abs(...)` | `sqrt` is internally emitted as `pow(arg, 0.5)`. |
| Energy Helpers | `W(...)`, `doublewell(...)` | Adds the canonical double-well potential `(u^2 - 1)^2`. |
| Variational Tools | `el(energy, variable)` (`EL`, `eulerlagrange`, `euler_lagrange`) | Computes the Euler–Lagrange residual for the given energy density and variable. |

Custom functions can be registered at runtime via `StringExpressionParser::registerFunction`, provided their derivatives are supplied when needed.
