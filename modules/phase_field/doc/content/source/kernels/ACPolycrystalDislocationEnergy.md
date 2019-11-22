# ACPolycrystalDislocationEnergy

!syntax description /Kernels/ACPolycrystalDislocationEnergy

`ACPolycrystalDislocationEnergy` implements the kernel for a bulk free energy density contribution from the presence of dislocations for the Allen-Cahn equation. The dislocation energy density contribution is described as [!cite](gentry2015simulating)

$f_{\mathrm{disloc}}\left(\eta_1, \eta_2, ... \eta_p\right) = \frac{1}{2}\mathrm{Gb}^2 \rho_{\mathrm{eff}}\left(\eta_1, \eta_2, ... \eta_p\right)$

where $G$ is the shear modulus of the material, $b$ is the Burgers vector, and $\rho_{\mathrm{eff}}$ is the effective dislocation density.  The effective local dislocation density, $\rho_{\mathrm{eff}}$, interpolates dislocation densities across diffuse interfaces for an arbitrary number of grains and is given by

$\rho_{\mathrm{eff}}\left(\eta_1, \eta_2, ... \eta_p \right) = \frac{\sum_i \rho_i \eta_i}{\sum_i \eta_i}$

where $\rho_i$ is the dislocation density of a given grain.  The effective dislocation density is computed in [PolycrystalDislocationDensity](/materials/PolycrystalDislocationDensity.md).

!syntax parameters /Kernels/ACPolycrystalDislocationEnergy

!syntax inputs /Kernels/ACPolycrystalDislocationEnergy

!syntax children /Kernels/ACPolycrystalDislocationEnergy

!bibtex bibliography
