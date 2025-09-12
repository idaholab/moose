# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Goal: Variational Derivatives for Phase-Field and Mechanics

This codebase computes **variational derivatives** (functional derivatives) symbolically, as needed in:
- **Phase-field modeling**: Chemical potential μ = δF/δc where F is the Gibbs energy functional
- **Interfacial mechanics**: Surface stress contributions from order parameter gradients
- **General field theories**: Any energy functional depending on fields and their gradients

### Mathematical Foundation

For a functional F[u] = ∫ f(x, u, ∇u, ∇²u, ...) dx, the variational derivative is:

```
δF/δu = Σ_{k=0}^m (-1)^k ∇^(k) : ∂f/∂(∇^(k)u)
```

Where:
- k=0: Direct dependence on field value u
- k=1: Dependence on gradient ∇u (requires -div of the partial)
- k=2: Dependence on Hessian ∇²u (requires +div² of the partial)
- Higher orders follow with alternating signs

## Build and Test Commands

```bash
# Build the test executable
make

# Run tests (builds if needed)  
make run

# Clean build artifacts
make clean

# Compiler and flags can be overridden
CXX=g++ make
CXXFLAGS="-std=c++17 -O3 -g" make
```

## Current Capabilities

### Core Variational Derivative Machinery

1. **Automatic Differentiation**: 
   - Computes ∂f/∂u, ∂f/∂(∇u), ∂f/∂(∇²u), etc. symbolically
   - Handles chain rule through complex compositions
   - Example: For f containing grad(c)/|grad(c)|, correctly computes ∂f/∂(∇c)

2. **Euler-Lagrange Assembly**:
   - Automatically constructs δF/δu from the partial derivatives
   - Applies k-fold divergences with proper alternating signs
   - Example: Cahn-Hilliard μ = W'(c) - ∇·(κ∇c) from f = W(c) + (κ/2)|∇c|²

3. **Normalized Gradient Handling**:
   - Critical for surface energies depending on interface normal n = ∇φ/|∇φ|
   - Implements ∂n/∂(∇φ) = (I - n⊗n)/|∇φ| correctly
   - Used in surface tension models: f = γ(n)|∇φ|

### Supported Operations and Their Derivatives

**Differential Operators**:
- `grad(u)`: Gradient (shifts derivative order up by 1)
- `div(v, k)`: k-fold divergence for Euler-Lagrange assembly
- Both handle the order-shifting properly in variational context

**Vector Operations** (essential for normalized gradients):
- `norm(v)`: |v|, derivative: v/|v| · dv
- `normalize(v)`: v/|v|, derivative: (I - n⊗n)dv/|v| where n = v/|v|
- `dot(a,b)`, `outer(a,b)`: With proper product rules

**Tensor Operations** (for mechanics applications):
- `matmul(A,B)`: Matrix multiplication
- `trace(A)`: For invariants like I₁ = tr(A)
- `det(A)`: For J = det(F) in finite deformation
- `inv(A)`: Matrix inverse, d(A⁻¹) = -A⁻¹(dA)A⁻¹
- `cofactor(A)`: det(A)·A⁻ᵀ, appears in mechanics
- `sym(A)`, `skew(A)`, `dev(A)`: Decompositions
- `transpose(A)`, `contract(A,B)`, `frobenius(A,B)`

**Function Registry** (extensible for specific energies):
- `W(c)`: Double-well potential, provides dW/dc
- `gamma(n)`: Surface energy coefficient γ(n)
- `I1, I2, I3`: Tensor invariants with correct derivatives
- `J(A)`: √det(A) for deformation measures
- Custom functions can be registered with derivative rules

## Architecture

1. **Typed Expression System**: 
   - Shape tags: `ScalarTag`, `VecTag<D>`, `MatTag<R,C>`, `TensorTag<R,D>`
   - `Expr<Tag>` wrapper enforces compile-time type safety
   - Prevents invalid operations (e.g., adding scalar to vector)

2. **AST Representation** (`include/ast.hpp`):
   - Node-based with `shared_ptr` for DAG structure
   - Each operation is a `Node` subclass
   - Symbolic manipulation without evaluation

3. **Differentiation Visitor** (`include/diff.hpp`):
   - `DiffVisitor` traverses AST computing derivatives
   - Returns `Differential` mapping order k → coefficient C^(k)
   - Implements all chain rules and special cases

4. **Pretty Printing** (`include/pretty.hpp`):
   - Human-readable symbolic output
   - Essential for debugging complex expressions

5. **Evaluation** (`include/eval.hpp`, `include/value_ops.hpp`):
   - Optional numerical backend
   - Used for finite-difference validation in tests

## Example Use Cases

### Phase-Field (Cahn-Hilliard)
```cpp
f = W(c) + (κ/2)|∇c|²
μ = δF/δc = W'(c) - ∇·(κ∇c)
```
The code correctly computes C⁰ = W'(c) and C¹ = κ∇c, then assembles the Euler-Lagrange equation.

### Surface Energy with Interface Normal
```cpp
n = ∇φ/|∇φ|
f = γ(n)|∇φ|
∂f/∂(∇φ) = γ(n)n + (∂γ/∂n)·(I - n⊗n)
```
Critical for models where surface tension depends on orientation.

### Mechanics with Deformation Gradient
```cpp
F = ∇u (deformation gradient)
W = μ/2(tr(FᵀF) - 3) + λ/2(ln J)²  (Neo-Hookean-like)
P = ∂W/∂F (first Piola-Kirchhoff stress)
```
The system handles these tensor derivatives correctly.

## Testing Philosophy

Tests validate:
- Symbolic correctness by checking string representations
- Numerical accuracy via finite-difference comparisons
- Complex compositions (normalized gradients, tensor operations)
- Higher-order derivatives (Hessians for phase-field models)

## Finite Element Integration (NEW)

The codebase now includes `include/fe_eval.hpp` for finite element evaluation with automatic variable splitting:

### Variable Splitting for Higher-Order Derivatives

When your FE system only provides values and gradients (and optionally Hessians), but the energy contains higher-order derivatives:
- **Automatic Detection**: `SplitAnalyzer` identifies which fields need higher derivatives than available
- **Variable Substitution**: Introduces auxiliary variables (e.g., q = ∇c) to rewrite ∇²c as ∇q
- **Transparent Transformation**: `SplitTransformer` rewrites expressions automatically

Example: For Cahn-Hilliard with 4th-order regularization f = W(c) + κ|∇c|² + λ|∇²c|²:
- System detects need for ∇²c
- Introduces q = ∇c as auxiliary variable
- Transforms ∇²c → ∇q in all expressions
- FE system solves coupled problem with c and q

### FE Evaluation Context

```cpp
FEEvaluator<2> evaluator;
evaluator.context.add_field("c", value, gradient);
evaluator.context.add_field_with_hessian("phi", value, gradient, hessian);

// Compute residual for FE assembly
auto contributions = evaluator.compute_contributions(energy_density, "c");
double residual = contributions.residual;
```

### Expression Simplification

Built-in simplification rules:
- Identity elimination: 0+x→x, 1*x→x, I·x→x
- Zero propagation: 0*x→0
- Constant folding: 2*3→6
- Common subexpression detection (future)

### Test Suite

Run FE-specific tests:
```bash
make test_fe       # Variable splitting and simplification tests
make test_assembly # Full FE assembly with shape functions
make test_galerkin # Galerkin FEM with test functions
```

## Galerkin FEM Integration (NEW)

The codebase now includes `include/galerkin_fem.hpp` for proper Galerkin finite element integration:

### Key Distinction: Test Functions vs Shape Functions

In Galerkin FEM:
- **Residual Assembly**: Uses test functions (`_test[_i][_qp]`, `_grad_test[_i][_qp]`)
- **Jacobian Assembly**: Uses shape functions (`_phi[_j][_qp]`, `_grad_phi[_j][_qp]`)

### Variable Declaration with Proper Accessors

```cpp
GalerkinSystem<2> system;
auto c = system.declareVariable("c",
    c_values,      // _c[_qp] - field values at quadrature points
    c_gradients,   // _grad_c[_qp] - field gradients
    test_values,   // _test[_i][_qp] - test functions for residual
    grad_test,     // _grad_test[_i][_qp] - test gradients
    phi_values,    // _phi[_j][_qp] - shape functions for Jacobian
    grad_phi       // _grad_phi[_j][_qp] - shape gradients
);
```

### Weak Form Assembly

The weak form for energy f(c, ∇c) with variational derivative δF/δc:

```cpp
// Strong form: δF/δc = C^0 - div(C^1) + div²(C^2) - ...

// Weak form (after integration by parts):
// R_i = ∫(C^0·test_i - C^1·∇test_i + C^2:∇²test_i - ...) dx

double residual_contribution = system.computeResidualContribution(df);
// This computes: C^0·test[i] - C^1·∇test[i] at current QP
```

### Example: Cahn-Hilliard Weak Form

For energy f = W(c) + (κ/2)|∇c|²:
- C^0 = dW/dc
- C^1 = κ∇c
- Weak form: R_i = ∫(dW/dc·test_i - κ∇c·∇test_i) dx

This avoids explicit divergence computation by using test functions and integration by parts.

## Complete FE Assembly System

Also includes `include/fe_assembly.hpp` for alternative assembly approach:

### Declaring Variables with Shape Functions

```cpp
// Your FE system provides these accessors at quadrature points:
// _c[qp], _grad_c[qp], _phi[qp], _grad_phi[qp]

FEAssembly<2> assembly;
FEAssembly<2>::VariableData c_data;
c_data.name = "c";
c_data.values = /* values at QPs */;
c_data.gradients = /* gradients at QPs */;
c_data.phi = /* shape functions [dof][qp] */;
c_data.grad_phi = /* shape function gradients [dof][qp] */;
c_data.dof_values = /* DOF values */;

auto c = assembly.declareVariable(c_data);
```

### Residual Assembly

```cpp
// Define energy
auto energy = Expr<ScalarTag>{
    add(fun("W", c), 
        mul(constant(0.5), mul(kappa, dot(grad(c), grad(c)))))
};

// Compute variational derivative
DiffVisitor dv{"c"};
auto df = dv.df(energy.n);
auto el = euler_lagrange(df);

// Assemble residual vector
auto residual = assembly.computeResidual(el);
```

### Jacobian Assembly

```cpp
// Compute Jacobian matrix using finite differences on DOFs
auto jacobian = assembly.computeJacobian(energy, "c");
```

### Key Features

1. **Proper Divergence Evaluation**: Uses shape function gradients to compute divergence at quadrature points
2. **DOF-Based Assembly**: Interpolates from DOF values to quadrature points using shape functions
3. **Test Function Integration**: Multiplies by test functions and integrates with JxW
4. **Normalized Gradient Support**: Correctly evaluates n = ∇φ/|∇φ| at quadrature points
5. **Jacobian via Finite Differences**: Computes Jacobian by perturbing DOFs (second variation support planned)

Run tests to see symbolic outputs and numerical validation results.