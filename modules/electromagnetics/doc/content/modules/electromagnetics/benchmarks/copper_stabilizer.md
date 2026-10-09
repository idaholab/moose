# Copper Stabilizer EM-Thermal Coupled Simulation

## Overview

This case implements and verifies a coupled electromagnetic-thermal
quench simulation framework for an infinitely long copper cylinder,
following the vector potential formulation of [!cite](merrill2015).

## Physics

### Electromagnetic equation

$$\mu_0\sigma(T)\frac{\partial A_z}{\partial t} =
\frac{1}{r}\frac{\partial}{\partial r}
\left(r\frac{\partial A_z}{\partial r}\right) + \mu_0 J_z$$

### Thermal equation

$$\rho C_p\frac{\partial T}{\partial t} =
\frac{1}{r}\frac{\partial}{\partial r}
\left(kr\frac{\partial T}{\partial r}\right) + Q(r,T)$$

### Coupling

$$Q(r,T) = J_z^2\,\rho_0(1 + \alpha(T-T_0)) \quad r \leq R$$

## Verification

| Quantity | L₂ error | Reference |
|---|---|---|
| Az | 0.01% | Eq. (1) |
| Bφ | 1.92% | Eq. (2) |
| T(r) profile | shape ✓ | Eq. (3) |
| Coupled ΔT | ~9 K | — |

## Input Files

| File | Purpose |
|---|---|
| `copper_stablizer1_B.i` | Magnetostatic verification |
| `copper_stablizer1_B_transient_1s.i` | Transient verification |
| `copper_stablizer1_B_transient_thermal.i` | Thermal verification |
| `copper_stablizer1_B_transient_thermal_coupling.i` | Coupled EM-thermal verification |

## References

!bibliography
