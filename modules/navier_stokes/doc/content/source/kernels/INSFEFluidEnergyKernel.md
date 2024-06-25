# INSFEFluidEnergyKernel

!syntax description /Kernels/INSFEFluidEnergyKernel

This kernel implements most terms in the conservation of energy equation for a continuous Galerkin discretization.
Both regular and porous media flow can be represented in this kernel, by specifying the [!param](/Kernels/INSFEFluidEnergyKernel/porosity) parameter.
The following terms are implemented:

- a convective term, modeling the advection of energy
- a diffusive term, including turbulence effects through the thermal conductivity computed by an [INSFEMaterial.md]
- a volumetric heat source term. It can be provided as a field using the [!param](/Kernels/INSFEFluidEnergyKernel/power_density)
  parameter or as a scalar variable (single value, uniform over the volume) with the [!param](/Kernels/INSFEFluidEnergyKernel/pke_power_var)


If selecting the conservative form using the [!param](/Kernels/INSFEFluidEnergyKernel/conservative_form) boolean
parameter, the convective term of the equation is expressed in the conservative form, i.e., $\nabla \cdot (\rho \vec{v} h)$, and integration by parts is applied to obtain the weak form.
Otherwise, the primitive form, i.e., $\rho c_p \vec{v} \cdot \nabla T$, is used, and integration by parts is **not** applied.

!syntax parameters /Kernels/INSFEFluidEnergyKernel

!syntax inputs /Kernels/INSFEFluidEnergyKernel

!syntax children /Kernels/INSFEFluidEnergyKernel
