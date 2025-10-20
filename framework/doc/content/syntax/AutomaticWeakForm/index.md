# Automatic Weak Form Generation

## Executive Summary

This document outlines the **completed integration** of automatic weak form generation from energy functionals into MOOSE. The system enables users to specify physics through energy functionals, automatically computes variational derivatives, generates appropriate kernels, and handles variable splitting for higher-order PDEs.

### Completed Components

#### 1. Core Action System
- **AutomaticWeakFormAction** (`framework/src/actions/AutomaticWeakFormAction.C`)
  - Parses energy expressions from input files
  - Analyzes variables and derivative orders
  - Generates appropriate kernels automatically
  - Handles variable splitting when needed
  - Manages coupled multi-physics problems

#### 2. Kernel Implementation
- **VariationalKernelBase** (`framework/src/kernels/VariationalKernelBase.C`)
  - Computes weak form residuals from energy functionals
  - Handles automatic differentiation for Jacobians
  - Supports coupled variables
  - Evaluates symbolic expressions at quadrature points

#### 3. Variable Splitting System
- **Constraint Kernels** for enforcing split variable relationships:
  - `GradientConstraintKernel` - General gradient constraints
  - `ScalarGradientConstraint` - Scalar gradient constraints (1D)
  - `VectorGradientConstraint` - Vector gradient constraints
  - `VariationalTimeDerivative` - Time derivatives for gradient flow

#### 4. Expression System
- **StringExpressionParser** - Parses mathematical expressions
- **ExpressionTransformer** - Transforms for variable splitting
- **ExpressionSimplifier** - Symbolic simplification
- **MooseAST** - Expression tree with differentiation support

### Working Features

#### Energy Expression Parsing
```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
  parameters = 'kappa 0.01'
  variables = 'c'
[]
```

Supports:
- Basic operations: `+`, `-`, `*`, `/`, `^`, `pow()`, `sqrt()`
- Differential operators: `grad()`, `div()`, `laplacian()`
- Vector operations: `dot()`, `cross()`, `norm()`, `normalize()`
- Tensor operations: `trace()`, `det()`, `inv()`, `transpose()`
- Special functions: `W()` (double-well), `exp()`, `log()`, `sin()`, `cos()`

#### Variable Splitting
Automatically handles higher-order PDEs:

```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  # 4th-order term triggers splitting
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*dot(grad(grad(c)), grad(grad(c)))'
  enable_splitting = true
  max_fe_order = 1  # Forces splitting
[]
```

System automatically:
- Detects derivatives exceeding FE capabilities
- Creates auxiliary variables (e.g., `c_hess = ∇²c`)
- Transforms energy expressions
- Generates constraint kernels

#### Coupled Multi-Physics
```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  # Coupled Cahn-Hilliard and thermal diffusion
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*k_th*dot(grad(T), grad(T)) + beta*c*T'
  parameters = 'kappa 0.01 k_th 1.0 beta 0.1'
  variables = 'c T'
[]
```

Automatically handles:
- Variable coupling based on energy expression
- Generates kernels for each variable
- Computes off-diagonal Jacobian contributions

### Test Cases

Working test cases in `test/tests/variational/`:

1. **Basic Cahn-Hilliard**
   - `cahn_hilliard_transient_ad.i` - Transient with AD
   - `cahn_hilliard_steady_ad.i` - Steady-state
   - `cahn_hilliard_no_ad.i` - Without AD

2. **Fourth-Order Problems**
   - `cahn_hilliard_fourth_order_split.i` - With variable splitting
   - `fourth_order_splitting.i` - Tests splitting mechanism
   - `test_biharmonic_split.i` - Biharmonic equation

3. **Coupled Problems**
   - `coupled_ch_diffusion_simple.i` - CH with thermal diffusion
   - `coupled_ch_mechanics_string.i` - CH with mechanics (simplified)

4. **Verification Tests**
   - `double_well_ad.i` - Double-well potential
   - `polynomial_ad.i` - Polynomial energy
   - `test_verbose_output.i` - Debug output

## Architecture Overview

### Directory Structure
```
framework/
├── include/
│   ├── actions/
│   │   └── AutomaticWeakFormAction.h
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
│   ├── actions/
│   │   └── AutomaticWeakFormAction.C
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
    ├── actions/
    │   └── AutomaticWeakFormAction.md
    └── kernels/
        ├── VariationalKernelBase.md
        ├── GradientConstraintKernel.md
        ├── ScalarGradientConstraint.md
        ├── VectorGradientConstraint.md
        └── VariationalTimeDerivative.md
```

### Workflow

1. **User Input**: Specify energy functional in input file
2. **Parsing**: AutomaticWeakFormAction parses energy expression
3. **Analysis**:
   - Identify variables and maximum derivative orders
   - Determine if splitting is needed
4. **Transformation**:
   - If splitting needed, introduce auxiliary variables
   - Transform energy expression
5. **Generation**:
   - Create primary variables and split variables
   - Generate VariationalKernelBase for each variable
   - Generate constraint kernels for split variables
6. **Execution**: Standard MOOSE solve

## Mathematical Foundation

### Variational Derivatives
For functional F[u] = ∫ f(u, ∇u, ∇²u, ...) dx:

```
δF/δu = ∂f/∂u - ∇·(∂f/∂∇u) + ∇²:(∂f/∂∇²u) - ...
```

### Weak Form
After integration by parts:

```
R = ∫ (C⁰·v - C¹·∇v + C²:∇²v - ...) dx
```

where C^k = ∂f/∂(∇^k u) are variational coefficients.

### Variable Splitting
For 4th-order term |∇²u|²:
- Introduce q = ∇²u
- Transform: |∇²u|² → |q|²
- Add constraint: q - ∇²u = 0 (enforced weakly)

## Usage Examples

### Simple Diffusion
```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = '0.5*dot(grad(u), grad(u))'
  variables = 'u'
[]
```

### Allen-Cahn Equation
```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = 'W(eta) + 0.5*kappa*dot(grad(eta), grad(eta))'
  parameters = 'kappa 0.5'
  variables = 'eta'
[]
```

### Fourth-Order Cahn-Hilliard
```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*dot(grad(grad(c)), grad(grad(c)))'
  parameters = 'kappa 0.01 lambda 0.001'
  variables = 'c'
  enable_splitting = true
  max_fe_order = 1
[]
```

## Testing and Validation

### Run Tests
```bash
cd test
make -j4

# Run all variational tests
./run_tests -i tests/variational

# Run specific test
./moose_test-opt -i tests/variational/cahn_hilliard_transient_ad.i
```

### Verification Methods
- Compare with analytical solutions
- Energy dissipation for gradient flow
- Conservation properties
- Convergence studies

## Known Limitations

1. **Tensor Mechanics**: Full tensor mechanics requires specialized expressions
2. **Nonlocal Terms**: Integral operators not directly supported
3. **Boundary Integrals**: Only volume integrals in energy functionals
4. **Performance**: Complex expressions may be slower than hand-coded kernels

## Future Enhancements

### Near-term
- [ ] Improved tensor mechanics support
- [ ] Optimized expression evaluation
- [ ] More predefined energy functionals
- [ ] Better error messages for expression syntax

### Long-term
- [ ] Nonlocal operators (integral terms)
- [ ] Boundary integral support
- [ ] Adaptive splitting strategies
- [ ] GPU acceleration for expression evaluation
- [ ] Integration with MOOSE's AD system

## Migration Guide

### For Existing MOOSE Users

Replace manual kernel implementation:

**Before** (manual):
```ini
[Kernels]
  [bulk]
    type = CahnHilliardBulk
    variable = c
    f_name = f_bulk
  []
  [interface]
    type = CahnHilliardInterface
    variable = c
    kappa = 0.01
  []
[]
```

**After** (automatic):
```ini
[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
  parameters = 'kappa 0.01'
  variables = 'c'
[]
```

### Benefits
- Fewer lines of code
- Automatic Jacobian computation
- Guaranteed consistency between residual and Jacobian
- Easy to modify energy functional

## Documentation

Comprehensive documentation available:
- `framework/doc/content/source/actions/AutomaticWeakFormAction.md`
- `framework/doc/content/source/kernels/VariationalKernelBase.md`
- `framework/doc/content/source/kernels/*GradientConstraint.md`

## Contributing

To extend the system:

1. **Add New Functions**:
   - Update `StringExpressionParser` to recognize function
   - Add differentiation rules in `MooseAST`
   - Create test case

2. **Add Energy Types**:
   - Add enum value in `AutomaticWeakFormAction`
   - Implement energy builder
   - Document in markdown

3. **Improve Performance**:
   - Profile expression evaluation
   - Implement caching/memoization
   - Consider compile-time optimization

## Support

For issues or questions:
- Create issue on GitHub
- Check test cases for examples
- Review documentation
- Enable `verbose = true` for debugging

## Conclusion

The automatic weak form generation system is **fully integrated** into MOOSE and provides:
- Automatic derivation of weak forms from energy functionals
- Variable splitting for higher-order PDEs
- Coupled multi-physics support
- Symbolic differentiation and simplification
- Comprehensive test coverage
- Full documentation

The system is ready for use in production simulations while continuing to evolve with additional features and optimizations.
