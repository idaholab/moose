# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview: Automatic Weak Form Generation for MOOSE

This project integrates automatic weak form generation from energy functionals into the MOOSE framework. It enables users to specify physics through energy functionals rather than manually deriving and implementing weak forms.

### Key Innovation

Instead of manually implementing kernels for PDEs, users can:
1. Specify an energy functional F[u]
2. The system automatically computes variational derivatives δF/δu
3. Weak form kernels are generated with correct residuals and Jacobians
4. Higher-order PDEs are automatically split into systems of lower-order equations

## MOOSE Integration

### Core Components

1. **AutomaticWeakFormAction** (`framework/src/actions/AutomaticWeakFormAction.C`)
   - Main action that orchestrates the automatic weak form generation
   - Parses energy expressions and generates appropriate kernels
   - Handles variable splitting for higher-order PDEs
   - Manages coupled multi-physics problems

2. **VariationalKernelBase** (`framework/src/kernels/VariationalKernelBase.C`)
   - Base kernel class that implements weak forms from energy functionals
   - Computes residuals and Jacobians using symbolic differentiation
   - Handles coupled variables automatically
   - Supports both predefined and custom energy expressions

3. **Constraint Kernels** (for variable splitting)
   - `GradientConstraintKernel`: General gradient constraints
   - `ScalarGradientConstraint`: Scalar gradient constraints (1D)
   - `VectorGradientConstraint`: Vector gradient constraints
   - `VariationalTimeDerivative`: Time derivatives for gradient flow

4. **Expression Transformation** (`framework/src/utils/automatic_weak_form/`)
   - `ExpressionTransformer`: Transforms expressions for variable splitting
   - `ExpressionSimplifier`: Simplifies symbolic expressions
   - `StringExpressionParser`: Parses mathematical expressions
   - `WeakFormGenerator`: Generates weak form contributions

## Mathematical Foundation

For a functional F[u] = ∫ f(x, u, ∇u, ∇²u, ...) dx, the variational derivative is:

```
δF/δu = Σ_{k=0}^m (-1)^k ∇^(k) : ∂f/∂(∇^(k)u)
```

The weak form residual becomes:
```
R = ∫ (C⁰·v - C¹·∇v + C²:∇²v - ...) dx
```

where C^k = ∂f/∂(∇^k u) are the variational derivative coefficients.

## Build and Test Commands

```bash
# Build the MOOSE test app with automatic weak form support
cd test
make -j4

# Run specific test cases
./moose_test-opt -i tests/variational/cahn_hilliard_transient_ad.i
./moose_test-opt -i tests/variational/fourth_order_splitting.i
./moose_test-opt -i tests/variational/coupled_ch_diffusion_simple.i

# Run all variational tests
./run_tests -i tests/variational
```

## Current Capabilities

### 1. Energy Expression Parsing

Supports comprehensive mathematical operations:
- **Basic**: `+`, `-`, `*`, `/`, `^`, `pow()`, `sqrt()`, `abs()`
- **Differential**: `grad()`, `div()`, `laplacian()`
- **Vector**: `dot()`, `cross()`, `norm()`, `normalize()`, `outer()`
- **Tensor**: `trace()`, `det()`, `inv()`, `transpose()`, `sym()`, `dev()`
- **Special**: `W()` (double-well), `gamma()` (surface energy)

### 2. Variable Splitting

Automatically handles higher-order PDEs:
- Detects when derivatives exceed FE capabilities
- Introduces auxiliary variables (e.g., q = ∇²u)
- Transforms energy expressions
- Generates constraint kernels

Example: 4th-order Cahn-Hilliard
```
Original: f = λ|∇²c|²
Split: q = ∇²c, f = λ|q|²
```

### 3. Coupled Multi-Physics

Handles coupled problems automatically:
- Variables coupled based on energy expression
- Each kernel receives necessary coupled variables
- Supports arbitrary coupling patterns

Example: Coupled Cahn-Hilliard and thermal diffusion
```
f = W(c) + κ/2|∇c|² + k/2|∇T|² + β·c·T
```

### 4. Automatic Differentiation

Multiple approaches for derivatives:
- Symbolic differentiation (exact)
- Automatic differentiation (runtime)
- Finite differences (fallback)

## Usage Examples

### Simple Cahn-Hilliard

```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
  parameters = 'kappa 0.01'
  variables = 'c'
  use_automatic_differentiation = true
[]
```

### Fourth-Order with Splitting

```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*dot(grad(grad(c)), grad(grad(c)))'
  parameters = 'kappa 0.01 lambda 0.001'
  variables = 'c'
  enable_splitting = true
  max_fe_order = 1  # Forces splitting
[]
```

### Coupled Phase-Field

```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*k_th*dot(grad(T), grad(T)) + beta*c*T'
  parameters = 'kappa 0.01 k_th 1.0 beta 0.1'
  variables = 'c T'
[]
```

## File Structure

```
framework/
├── include/
│   ├── actions/AutomaticWeakFormAction.h
│   ├── kernels/
│   │   ├── VariationalKernelBase.h
│   │   ├── GradientConstraintKernel.h
│   │   ├── ScalarGradientConstraint.h
│   │   ├── VectorGradientConstraint.h
│   │   └── VariationalTimeDerivative.h
│   └── utils/automatic_weak_form/
│       ├── MooseAST.h
│       ├── StringExpressionParser.h
│       ├── ExpressionTransformer.h
│       └── ExpressionSimplifier.h
├── src/
│   ├── actions/AutomaticWeakFormAction.C
│   ├── kernels/
│   │   ├── VariationalKernelBase.C
│   │   ├── GradientConstraintKernel.C
│   │   ├── ScalarGradientConstraint.C
│   │   ├── VectorGradientConstraint.C
│   │   └── VariationalTimeDerivative.C
│   └── utils/automatic_weak_form/
│       ├── WeakFormGenerator.C
│       ├── StringExpressionParser.C
│       ├── ExpressionTransformer.C
│       └── ExpressionSimplifier.C
└── doc/content/source/
    ├── actions/AutomaticWeakFormAction.md
    └── kernels/
        ├── VariationalKernelBase.md
        ├── GradientConstraintKernel.md
        ├── ScalarGradientConstraint.md
        ├── VectorGradientConstraint.md
        └── VariationalTimeDerivative.md

test/
├── tests/variational/
│   ├── cahn_hilliard_*.i          # Cahn-Hilliard tests
│   ├── fourth_order_*.i           # Variable splitting tests
│   ├── coupled_*.i                # Multi-physics tests
│   └── test_*.i                   # Various test cases
└── src/base/MooseTestApp.C        # Registers new objects
```

## Debugging Tips

### Enable Verbose Output

```ini
[AutomaticWeakForm]
  verbose = true  # Shows parsed expressions, transformations, generated kernels
[]
```

### Check Generated Kernels

The verbose output shows:
- Parsed energy expression
- Generated split variables
- Transformed expressions
- Created kernels and couplings

### Common Issues

1. **Convergence problems with splitting**
   - Check constraint scaling
   - Verify boundary conditions for auxiliary variables
   - Use appropriate preconditioners for saddle-point systems

2. **Coupled variable errors**
   - Ensure all variables in energy expression are listed in `variables`
   - Check parameter syntax (space-separated key-value pairs)

3. **Expression parsing errors**
   - Verify function names (e.g., `dot()` not `·`)
   - Check parentheses matching
   - Ensure parameters are defined

## Testing

Run the test suite:
```bash
cd test
./run_tests -i tests/variational
```

Key test files:
- `cahn_hilliard_transient_ad.i`: Basic Cahn-Hilliard with AD
- `fourth_order_splitting.i`: Tests variable splitting
- `coupled_ch_diffusion_simple.i`: Coupled multi-physics
- `test_verbose_output.i`: Shows detailed debug output

## Future Enhancements

Planned improvements:
- [ ] Tensor mechanics with proper strain measures
- [ ] Nonlocal operators (integral terms)
- [ ] Adaptive variable splitting strategies
- [ ] Performance optimizations for complex expressions
- [ ] Support for boundary integrals in energy functionals

## Contributing

When adding new features:
1. Update the expression parser for new operations
2. Add differentiation rules in MooseAST
3. Create test cases in `tests/variational/`
4. Update documentation in `framework/doc/`
5. Ensure backward compatibility

## References

Key papers and resources:
- Variational principles in continuum mechanics
- Phase-field modeling literature
- MOOSE documentation: https://mooseframework.inl.gov
- Automatic differentiation techniques