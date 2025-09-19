# Step 4: Materials id=ictp_step4

!---

## Materials

Realistic problems have spatially dependent properties. Often these properties are also shared across equations (or Variables, in MOOSE world).

In MOOSE, these properties are defined using the [`Materials`](Materials/index.md) system:

- Material objects produce one or more named "material properties" on quadrature points
- The values are computed on-demand (during residual and Jacobian evaluation)
- Material properties can be of any type (scalars, vector values, etc)
- Can be restricted to blocks for spatial variation
- Can consume variable values (ex: fluid properties depend on $T$)
- Can depend on other materials (adds execution dependencies)
- Can optionally propagate derivatives through automatic differentiation

Objects like Kernels then consume material properties. This pluggable design enables the convenient sharing of material properties in a well-defined manner.

!---

## Spatially-Dependent Diffusion

We will take our previous diffusion solve and introduce coefficients of the diffusivity to the fuel region (block `fuel`, of value $100$) and the cladding region (block `clad`, of value $100$). The previous coefficient of diffusivity was $1$ for both regions.

This will require two changes:

- Replacing the [`Diffusion`](Diffusion.md) kernel with a [`MatDiffusion`](MatDiffusion.md) kernel, which consumes a material property named `D` for the diffusion coefficient
- Adding two [`GenericConstantMaterial`](GenericConstantMaterial.md) Material objects, one for each of the two blocks

!---

## Input: Spatially-Dependent Diffusion

!listing moose/step4_diffusion_materials.i diff=moose/step2-1_diffusion.i

!---

## Run: Spatially-Dependent Diffusion

```bash
cardinal-opt -i step4_diffusion_materials.i
```

!---

## Result: Spatially-Dependent Diffusion

!style halign=center
!media step4-1_solution.png style=width:50%
