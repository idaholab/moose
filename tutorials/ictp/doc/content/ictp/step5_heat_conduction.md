# Step 5: Heat Conduction id=ictp_step5

!---

## Solid Heat Conduction

Thus far, we've solved the diffusion problem on our pin cell with heterogeneity between the clad and the fuel. Let's solve something a little more realistic. We will solve a heat conduction problem on the same solid geometry where:

- The outer boundary (`water_solid_interface`) has a prescribed temperature of $300$ K
- The inner boundary has zero heat flux
- There is a volumetric heat source in the fuel (`fuel`) of $10^8~\text{W}/\text{m}^2$

The material properties are as follows:

| Property | Fuel | Clad |
| - | - | - |
| $k$ | $2~\text{W}/\text{m}^2$ | $10~\text{W}/\text{m}^2$ |
| $c_p$ | $3100~\text{W}/(\text{K} \cdot \text{kg})$ | $2800~\text{W}/(\text{K} \cdot \text{kg})$ |
| $\rho$ | $10700~\text{kg}/\text{m}^3$ | $5400~\text{kg}/\text{m}^3$ |

!---

## Additional Capabilities

A few new things will be introduced in this step:

- [`Physics`](Physics/index.md) system: for setting up a heat conduction problem using the convenience [`Physics/HeatConduction/FiniteElement`](physics/HeatConductionCG.md) syntax
- [`AuxVariable`](AuxVariables/index.md) system: for defining fields that are auxiliary (not solved for)
- [`AuxKernel`](AuxKernels/index.md) system: for filling an `AuxVariable`

!---

## Auxiliary Variables and Kernels

The term "auxiliary variable" is defined, in MOOSE language, as a variable that is directly calculated using an `AuxKernel` object. An `AuxKernel` fills into an `AuxVariable`. This allows for postprocessing, coupling, and proxy calculations.

Auxiliary variables come in two flavors:

- Elemental: constant or higher-order monomials; discontinuous across elements
- Nodal: like linear-lagrange; continuous across elements

In particular, we will utilize the [`DiffusionFluxAux`](DiffusionFluxAux.md) to compute the integral of the heat flux on the outer boundary (`water_solid_interface`).

We will also use an `AuxVaraible` named `T_fluid` to define the field that is used as the outer boundary condition.

!---

## Input: Solid Heat Conduction

!listing ictp/inputs/step5_heat_conduction/solid.i

!---

## Run: Solid Heat Conduction

!---

```bash
$ cd ../step5_heat_conduction
$ cardinal-opt -i solid.i
```

```
Postprocessor Values:
+----------------+----------------+--------------------+
| time           | T_max          | heat_flux_integral |
+----------------+----------------+--------------------+
|   0.000000e+00 |   0.000000e+00 |       0.000000e+00 |
|   1.000000e+00 |   3.636917e+02 |       2.421074e+03 |
+----------------+----------------+--------------------+
```

!---

## Result: Solid Heat Conduction

!style halign=center
!media step5-1_solution.png style=width:50%

!style halign=center
From `solid_out.e` in Paraview

!---

## Result: Solid Heat Conduction Line

!style halign=center
!media step5-1_line.png style=width:50%

!style halign=center
Line plot of $T$ through $y = 0$ from `solid_out.e` in Paraview with refinement

!---

## Fluid Heat Conduction

Recall our "fluid" mesh from [#ictp_step1]:

!style halign=center
!media step1-4_mesh.png style=width:35%

!---

## Fluid Heat Conduction

It will become clear why in the next step -- but let's also solve a heat conduction problem on our fluid mesh with the following:

- The outer boundary (`outer`) has a prescribed temperature of $300~\text{K}$
- The inner boundary (`water_solid_interface`) has a Neumann boundary condition that fixes the incoming heat flux to a value of $2 \times 10^4~\text{W}/\text{m}^2$

The material properties are as follows:

| Property | Value |
| - | - | - |
| $k$ | $0.6~\text{W}/\text{m}^2$ |
| $c_p$ | $1000~\text{W}/(\text{K} \cdot \text{kg})$ |
| $\rho$ | $4100~\text{kg}/\text{m}^3$ |

!---

## Input: Fluid Heat Conduction

!listing ictp/inputs/step5_heat_conduction/fluid.i
