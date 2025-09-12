# MOOSE Integration Plan for Variational Derivative System

## Executive Summary

This document outlines the integration of the variational derivative symbolic computation system into MOOSE to enable automatic weak form generation from strong form energy functionals, including automatic variable splitting for higher-order derivatives.

## Architecture Overview

### 1. Core Components to Integrate

#### Phase 1: Core Symbolic System (Move to `framework/`)
- `include/utils/variational_derivative/AST.h` - Expression AST with typed nodes
- `include/utils/variational_derivative/Differentiation.h` - Variational derivative engine
- `include/utils/variational_derivative/VariableSplitting.h` - Automatic splitting for higher-order terms
- `include/utils/variational_derivative/WeakFormGenerator.h` - Generate weak form from strong form

#### Phase 2: MOOSE Integration Layer
- `include/actions/VariationalDerivativeAction.h` - Parse and setup system
- `include/kernels/VariationalKernel.h` - Base kernel for variational problems
- `include/kernels/SplitVariableKernel.h` - Kernels for auxiliary split variables

### 2. Proposed MOOSE Action Workflow

```cpp
[VariationalProblem]
  [energy]
    type = CahnHilliardEnergy
    variable = c
    # Energy: W(c) + (κ/2)|∇c|² + (λ/2)|∇²c|²
    double_well = 'W'
    kappa = 1.0
    lambda = 0.1  # Triggers 4th-order split
  []
[]
```

The `VariationalDerivativeAction` will:

1. **Parse Energy Functional**
   - Build AST from input parameters
   - Identify primary and coupled variables

2. **Compute Variational Derivatives**
   - Generate δF/δu for each variable
   - Extract C^(k) coefficients

3. **Analyze for Variable Splitting**
   - Detect max derivative order needed
   - Compare with available FE order
   - Generate split variables (e.g., q = ∇c for ∇²c terms)

4. **Add Variables and Kernels**
   ```cpp
   // Automatically generated:
   addVariable("c");           // Primary variable
   addAuxVariable("c_grad");   // Split variable q = ∇c
   
   addKernel("VariationalKernel", "c_residual");
   addKernel("SplitGradientKernel", "c_grad_compute");
   addKernel("SplitDiffusionKernel", "c_fourth_order");
   ```

### 3. Kernel Implementation Strategy

#### Base Variational Kernel
```cpp
class VariationalKernel : public Kernel
{
public:
  VariationalKernel(const InputParameters & parameters);
  
protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  
private:
  // Holds the differential coefficients
  std::unique_ptr<Differential> _diff;
  
  // C^0 contribution: C^0 * test
  Real computeC0Contribution();
  
  // C^1 contribution: -C^1 · ∇test
  Real computeC1Contribution();
  
  // C^2 contribution: C^2 : ∇²test (if needed)
  Real computeC2Contribution();
};
```

#### Split Variable Kernels
```cpp
class SplitGradientKernel : public AuxKernel
{
  // Computes q = ∇c at nodes
protected:
  virtual Real computeValue() override {
    return _grad_u[_qp](component);
  }
};

class SplitDiffusionKernel : public Kernel
{
  // Handles ∇²c terms via split variable q
  // Contributes: -∇q · ∇test to residual
};
```

### 4. Coupled Multi-Physics Support

For coupled systems like Cahn-Hilliard with mechanics:

```cpp
[VariationalProblem]
  [energy]
    type = CoupledCahnHilliardElasticity
    variables = 'c disp_x disp_y'
    
    # f = f_ch(c, ∇c) + f_elastic(ε(u), c)
    chemical_energy = 'W(c) + 0.5*kappa*|grad(c)|^2'
    elastic_energy = '0.5*lambda*tr(eps)^2 + mu*|eps|^2'
    coupling_energy = 'A*c*tr(eps)'
  []
[]
```

The system will generate:
- On-diagonal Jacobians: ∂R_c/∂c, ∂R_u/∂u
- Off-diagonal Jacobians: ∂R_c/∂u, ∂R_u/∂c

### 5. Implementation Phases

#### Phase 1: Core Integration (Weeks 1-2)
- Move symbolic system to MOOSE framework
- Create basic VariationalDerivativeAction
- Implement simple test cases (diffusion, Cahn-Hilliard)

#### Phase 2: Variable Splitting (Weeks 3-4)
- Implement automatic split detection
- Create split variable kernels
- Test with 4th-order Cahn-Hilliard

#### Phase 3: Coupled Systems (Weeks 5-6)
- Extend to multi-variable problems
- Implement off-diagonal Jacobians
- Test with coupled physics

#### Phase 4: Advanced Features (Weeks 7-8)
- Surface energy terms (normalized gradients)
- Tensor-valued problems (mechanics)
- Performance optimization

### 6. Testing Strategy

#### Unit Tests (`test/tests/kernels/variational/`)
- Test each C^k contribution separately
- Verify Jacobian accuracy with MMS
- Test variable splitting logic

#### Integration Tests
- Compare with existing MOOSE kernels
- Verify conservation properties
- Benchmark performance

#### Example Test Cases
1. **Simple Diffusion**: f = (1/2)|∇u|²
2. **Cahn-Hilliard**: f = W(c) + (κ/2)|∇c|²
3. **4th-Order CH**: f = W(c) + (κ/2)|∇c|² + (λ/2)|∇²c|²
4. **Coupled CH-Elasticity**: Chemical + mechanical energy
5. **Surface Energy**: f = γ(n)|∇φ| with n = ∇φ/|∇φ|

### 7. Input File Design

#### Simple Case
```
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
[]

[VariationalProblem]
  [./cahn_hilliard]
    type = VariationalDerivativeAction
    variable = c
    energy = 'double_well + gradient'
    
    [./double_well]
      expression = '(c^2 - 1)^2'
    [../]
    
    [./gradient]
      expression = '0.5 * kappa * dot(grad(c), grad(c))'
      kappa = 1.0
    [../]
  [../]
[]

[BCs]
  # Automatically handles natural BCs from variational form
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
[]
```

#### With Variable Splitting
```
[VariationalProblem]
  [./fourth_order]
    type = VariationalDerivativeAction
    variable = c
    energy_density = 'W(c) + 0.5*kappa*|grad(c)|^2 + 0.5*lambda*|grad(grad(c))|^2'
    
    # Automatically generates:
    # - AuxVariable: c_grad (= ∇c)
    # - Coupled kernels for 4th-order system
    
    auto_split = true
    max_fe_order = 1  # Forces splitting for 2nd derivatives
  [../]
[]
```

### 8. Benefits of Integration

1. **Automatic Consistency**: Weak form automatically derived from energy
2. **Reduced Code Duplication**: One energy definition generates all kernels
3. **Easier Multi-Physics**: Coupling terms handled automatically
4. **Variable Splitting**: Higher-order PDEs handled transparently
5. **Optimization Opportunities**: Symbolic simplification before evaluation

### 9. Migration Path

1. **Create MOOSE Fork/Branch**
   ```bash
   git checkout -b variational-derivative-system
   ```

2. **Directory Structure**
   ```
   framework/
     include/
       utils/
         variational_derivative/
           AST.h
           Differentiation.h
           VariableSplitting.h
       actions/
         VariationalDerivativeAction.h
       kernels/
         variational/
           VariationalKernel.h
           SplitVariableKernel.h
   ```

3. **Initial PR Strategy**
   - PR 1: Core symbolic system (utils/)
   - PR 2: Basic action and kernels
   - PR 3: Variable splitting
   - PR 4: Coupled systems support

### 10. Documentation Requirements

- Theory manual section on variational derivatives
- User guide for VariationalDerivativeAction
- Developer docs for extending energy functionals
- Example problems demonstrating capabilities

## Conclusion

This integration will provide MOOSE users with a powerful system for automatically deriving weak forms from energy functionals, handling complex multi-physics problems with minimal code while ensuring mathematical consistency.