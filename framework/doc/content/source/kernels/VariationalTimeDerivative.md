# VariationalTimeDerivative

!syntax description /Kernels/VariationalTimeDerivative

## Overview

`VariationalTimeDerivative` is a specialized time derivative kernel for variational problems. It implements the time derivative term in the weak form of PDEs derived from energy functionals, particularly for gradient flow and energy minimization problems.

## Purpose

This kernel is designed for:
- Gradient flow problems (Allen-Cahn, Cahn-Hilliard)
- Energy minimization dynamics
- Phase-field evolution equations
- Problems where time evolution follows variational principles

## Theory

### Gradient Flow

Many phase-field models follow gradient flow dynamics:

```
∂u/∂t = -M δF/δu
```

where:
- `M` is the mobility
- `F[u]` is the energy functional
- `δF/δu` is the variational derivative

### Weak Form

The time derivative term in weak form:

```
∫_Ω (∂u/∂t)·v dx
```

where `v` is the test function.

### Energy Dissipation

Gradient flow ensures energy decrease:

```
dF/dt = ∫_Ω (δF/δu)·(∂u/∂t) dx ≤ 0
```

## Mathematical Formulation

### Standard Form

For a simple time derivative:

```
Residual = (u_new - u_old)/Δt · test
```

### With Mobility

For gradient flow with mobility M(u):

```
Residual = (1/M(u)) · (u_new - u_old)/Δt · test
```

### L² Gradient Flow

For L² gradient flow (Allen-Cahn type):

```
∂u/∂t = -δF/δu
Residual = ∂u/∂t · test + (δF/δu) · test
```

### H⁻¹ Gradient Flow

For H⁻¹ gradient flow (Cahn-Hilliard type):

```
∂u/∂t = ∇·(M∇(δF/δu))
Residual = ∂u/∂t · test - M∇(δF/δu) · ∇test
```

## Implementation

```cpp
Real VariationalTimeDerivative::computeQpResidual()
{
  Real time_derivative = _u_dot[_qp];

  if (_gradient_flow_type == L2)
  {
    // Standard L² gradient flow
    return time_derivative * _test[_i][_qp];
  }
  else if (_gradient_flow_type == H1)
  {
    // H⁻¹ gradient flow (conserved dynamics)
    return time_derivative * _test[_i][_qp];
    // Chemical potential handled separately
  }

  // With mobility
  return time_derivative / _mobility[_qp] * _test[_i][_qp];
}
```

## Syntax

!syntax parameters /Kernels/VariationalTimeDerivative

## Input Parameters

### Required Parameters

- `variable`: The evolving field variable

### Optional Parameters

- `mobility`: Mobility coefficient or function
- `gradient_flow_type`: Type of gradient flow (L2, H1, custom)
- `time_stepping_scheme`: Implicit, explicit, or adaptive
- `lumping`: Use mass lumping for explicit schemes

## Examples

### Example 1: Allen-Cahn Equation

L² gradient flow for non-conserved dynamics:

```
[Kernels]
  # Time derivative
  [eta_dot]
    type = VariationalTimeDerivative
    variable = eta
    gradient_flow_type = L2
  []

  # Variational derivative of energy
  [eta_residual]
    type = VariationalKernelBase
    variable = eta
    energy_expression = 'W(eta) + 0.5*kappa*dot(grad(eta), grad(eta))'
  []
[]
```

### Example 2: Cahn-Hilliard Equation

H⁻¹ gradient flow for conserved dynamics:

```
[Variables]
  [c]  # Concentration
  []
  [mu]  # Chemical potential
  []
[]

[Kernels]
  # Time derivative
  [c_dot]
    type = VariationalTimeDerivative
    variable = c
    gradient_flow_type = H1
  []

  # Flux divergence
  [flux]
    type = MatDiffusion
    variable = c
    v = mu
    diffusivity = mobility
  []

  # Chemical potential from energy
  [chemical_potential]
    type = VariationalKernelBase
    variable = mu
    energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
  []
[]
```

### Example 3: Anisotropic Mobility

Phase-field with orientation-dependent mobility:

```
[Materials]
  [mobility]
    type = ParsedMaterial
    property_name = M
    expression = 'M0 * (1 + epsilon * cos(4*atan2(grad_eta_y, grad_eta_x)))'
    material_property_names = 'M0 epsilon'
    coupled_variables = 'eta'
  []
[]

[Kernels]
  [aniso_time]
    type = VariationalTimeDerivative
    variable = eta
    mobility = M  # Uses material property
  []
[]
```

### Example 4: Adaptive Time Stepping

With energy-based adaptivity:

```
[Kernels]
  [adaptive_time]
    type = VariationalTimeDerivative
    variable = phi
    time_stepping_scheme = adaptive
    target_energy_change = 1e-6
  []
[]

[Executioner]
  type = Transient

  [TimeSteppers]
    [energy_based]
      type = IterationAdaptiveDT
      optimal_iterations = 6
      growth_factor = 1.2
      cutback_factor = 0.5
    []
  []
[]
```

## Relationship to Standard TimeDerivative

### Differences from TimeDerivative

| Feature | TimeDerivative | VariationalTimeDerivative |
|---------|---------------|--------------------------|
| Purpose | General PDEs | Variational problems |
| Mobility | Fixed coefficient | Variable/anisotropic |
| Energy | Not considered | Ensures dissipation |
| Conservation | Optional | Built-in for H⁻¹ flow |

### When to Use

Use `VariationalTimeDerivative` when:
- Problem derives from energy functional
- Need to ensure energy dissipation
- Mobility is variable or anisotropic
- Want specialized gradient flow dynamics

## Time Integration Schemes

### Implicit Euler (default)

```
(u^{n+1} - u^n)/Δt = -M δF/δu|_{u^{n+1}}
```

Unconditionally stable but first-order accurate.

### Crank-Nicolson

```
(u^{n+1} - u^n)/Δt = -M/2 (δF/δu|_{u^{n+1}} + δF/δu|_{u^n})
```

Second-order accurate, conditionally stable.

### Energy-Stable Schemes

Special schemes that guarantee energy decrease:

```
[Executioner]
  type = Transient
  scheme = 'energy-stable'

  [TimeIntegrator]
    type = ImplicitMidpoint
    # Guarantees energy stability
  []
[]
```

## Numerical Considerations

### Mass Lumping

For explicit schemes, mass lumping improves stability:

```
[Kernels]
  [lumped_time]
    type = VariationalTimeDerivative
    variable = u
    lumping = true  # Diagonal mass matrix
  []
[]
```

### Adaptive Time Stepping

Monitor energy change for adaptivity:

```
[Postprocessors]
  [energy]
    type = ElementIntegralVariablePostprocessor
    variable = energy_density
  []

  [energy_change]
    type = ChangeOverTimePostprocessor
    postprocessor = energy
  []
[]

[Executioner]
  [TimeSteppers]
    [adaptive]
      type = PostprocessorDT
      postprocessor = energy_change
      scale = 0.1
    []
  []
[]
```

## Conservation Properties

### Mass Conservation

For conserved order parameters:

```
[Postprocessors]
  [total_mass]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial timestep_end'
  []

  [mass_balance]
    type = ChangeOverTimePostprocessor
    postprocessor = total_mass
    # Should be ~0 for conserved dynamics
  []
[]
```

### Energy Dissipation

Verify energy decreases:

```
[Postprocessors]
  [total_energy]
    type = ElementIntegralVariablePostprocessor
    variable = energy_density
  []

  [energy_rate]
    type = ChangeOverTimePostprocessor
    postprocessor = total_energy
    # Should be ≤ 0 for gradient flow
  []
[]
```

## Common Issues

### Issue: Energy Increases

**Solution**:
- Check time step size
- Verify mobility is positive
- Use energy-stable time integration

### Issue: Oscillations

**Solution**:
- Reduce time step
- Add numerical damping
- Use implicit schemes

### Issue: Slow Convergence

**Solution**:
- Use adaptive time stepping
- Precondition with mass matrix
- Consider semi-implicit schemes

## Advanced Features

### Multiple Time Scales

For problems with fast and slow dynamics:

```
[Kernels]
  [fast_dynamics]
    type = VariationalTimeDerivative
    variable = fast_var
    time_scale = 1e-3
  []

  [slow_dynamics]
    type = VariationalTimeDerivative
    variable = slow_var
    time_scale = 1.0
  []
[]
```

### Inertial Terms

For dynamic problems with inertia:

```
[Kernels]
  [inertial_time]
    type = VariationalTimeDerivative
    variable = u
    second_order = true  # ∂²u/∂t²
    mass_density = rho
  []
[]
```

## See Also

- [TimeDerivative.md] - Standard time derivative kernel
- [VariationalKernelBase.md] - Spatial terms from energy
- [AutomaticWeakFormAction.md] - Automatic generation
- [AllenCahn.md] - Specific Allen-Cahn implementation
- [CahnHilliard.md] - Specific Cahn-Hilliard implementation

!syntax children /Kernels/VariationalTimeDerivative