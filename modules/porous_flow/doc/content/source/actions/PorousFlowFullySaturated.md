# PorousFlowFullySaturated

!syntax description /PorousFlowFullySaturated/PorousFlowFullySaturated

For a worked example, see [tutorial page 6](/tutorial_06.md).

## Dictator

This `Action` automatically adds an appropriately-parameterized [PorousFlowDictator](PorousFlowDictator.md) with name given by the `dictator_name` input.

## Fluid equations

### Order of unknowns

The unknowns for the fluid equations are
\begin{equation}
\{\chi^{0}, \chi^{1}, \ldots, \chi^{N_{f}-2}, P\}
\end{equation}
Here $N_{f}$ is the number of fluid components, $\chi^{i}$ is a fluid component, and $P$ is the porepressure.  For example:

- For single-component fluids ($N_{f} = 1$) such as pure water, $P$ is the only variable
- For brine, whose fluid components are NaCl and H$_{2}$O, the variables are $\{\chi^{0}, P\} = \{\chi^{\mathrm{NaCl}}, P\}$
- For an acidic brine, whose basis species are Na$^{+}$, Cl$^{-}$, H$^{+}$ and H$_{2}$O (with $N_{f} = 4$), the variables are $\{\chi^{0}, \chi^{1}, \chi^{2}, P\} = \{\chi^{\mathrm{Na+}}, \chi^{\mathrm{Cl-}}, \chi^{\mathrm{H+}}, P\}$

Note that this `Action` associates the final mass-fraction with the porepressure variable.  Ordering your mass-fraction variables so that the final variable has the greatest mass-fraction will usually improve convergence speed because the Jacobian is more diagonally dominant.  For instance, for the brine situation, it is usually much more computationally efficient to choose $\chi^{0} = \chi^{\mathrm{NaCl}}$ rather than $\chi^{0} = \chi^{\mathrm{H2O}}$ even though the final answers will be identical.

Since the final mass-fraction, $1 - \sum_{\kappa = 0}^{N_{f} - 2}\chi^{\kappa}$, is associated with the porepressure variable, `BCs` and other objects associated with the porepressure variable will be "acting on" the final mass-fraction.  See [tutorial page 6](/tutorial_06.md) for examples.

### Fluid-flow equations

This `Action` simulates the $N_{f}$ [fluid equations](porous_flow/governing_equations.md)
\begin{equation}
\label{eq:full_sat}
0 = \frac{\partial}{\partial t} \phi(\rho) \chi^{\kappa} + \phi(\rho) \chi^{\kappa}\nabla\cdot \mathrm{v}_{s} - \nabla_{i} \left(\chi^{\kappa}(\rho)\frac{k_{ij}}{\mu}(\nabla P_{j} - \rho g_{j}) \right) \ .
\end{equation}
In this equation, the fluid density, $\rho$, appears in parenthases, because the user has the option of including it or not using the `multiply_by_density` flag.

!alert note
Think carefully before you use `multiply_by_density = false`.  When not multiply by density, in many situations the sum of [eq:full_sat] will imply a Laplace equation such $0 = \nabla (k(\nabla P - \rho g))$, that is a steady-state distribution for the porepressure.  In addition, care must be taken when using other parts of PorousFlow, for instance, the [`PorousFlowMass`](PorousFlowFluidMass.md) Postprocessor is coded to record fluid mass, not fluid volume.  New users should set `multiply_by_density = true` to avoid confusion, even at the expense of extra computational cost.

### Kernels added

To represent the $\frac{\partial}{\partial t} \phi(\rho) \chi^{\kappa}$ term (which only appears in `transient` simulations) the `Action` adds a [PorousFlowMasstimeDerivative](PorousFlowMassTimeDerivative.md) Kernel for each fluid component.  These Kernels lump the fluid-component mass to the nodes to ensure superior [numerical stabilization](stabilization.md).

To represent the $\phi(\rho) \chi^{\kappa}\nabla\cdot \mathrm{v}_{s}$ term (which only appears in `transient` simulations with mechanical `coupling_type`) the `Action` adds a [PorousFlowMassVolumetricExpansion](PorousFlowMassVolumetricExpansion.md) Kernel for each fluid component.  These Kernels lump the fluid-component mass to the nodes to ensure superior [numerical stabilization](stabilization.md).

To represent the $\nabla_{i} \left(\chi^{\kappa}(\rho)\frac{k_{ij}}{\mu}(\nabla P_{j} - \rho g_{j}) \right)$ the `Action` adds Kernels and possibly UserObjects depending on the `stabilization` parameter:

- In the case of no-upwinding, a [PorousFlowFullySaturatedDarcyFlow](PorousFlowFullySaturatedDarcyFlow.md) Kernel for each fluid component.
- For full upwinding, a [PorousFlowFullySaturatedAdvectiveFlux](PorousFlowFullySaturatedAdvectiveFlux.md) Kernel for each fluid component.
- For [KT stabilization](kt.md), a [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernel and a [PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent](PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent.md) UserObject for each fluid component (or a [PorousFlowAdvectiveFluxCalculatorSaturated](PorousFlowAdvectiveFluxCalculatorSaturated.md) in the case of just one fluid component).

### Advanced additions: diffusion and dispersion, radioactive decay, chemistry

The `Action` does *not* add Kernels to model the following effects

- Dispersion: if dispersion is important, the user must manually add a set of [PorousFlowDispersiveFlux](PorousFlowDispersiveFlux.md) Kernels.
- Radioactive decay: if this is important, the user must manually add some [PorousFlowMassRadioactiveDecay](PorousFlowMassRadioactiveDecay.md) Kernels.
- Chemistry: if chemistry is of importance, then appropriate Kernels must be manually added, such as [PorousFlowPreDis](PorousFlowPreDis.md), [PorousFlowDesorpedMassTimeDerivative](PorousFlowDesorpedMassTimeDerivative.md) and [PorousFlowDesorpedMassVolumetricExpansion](PorousFlowDesorpedMassVolumetricExpansion.md).  A simple example is discussed in the [tutorial 07 page](tutorial_07.md).  Alternatively, MOOSE's Geochemistry module may be employed.


## Heat equation

The [heat equation](governing_equations.md) is associated with the temperature variable, and is only included if the `coupling_type` includes "Thermo".
\begin{equation}
\label{eq:heat_cons}
0 = \frac{\partial}{\partial t}\left( (1 - \phi)\rho_{R}C_{R}T + \phi\rho\mathcal{E} \right) + \left( (1 - \phi)\rho_{R}C_{R}T + \phi\rho\mathcal{E} \right)\nabla\cdot{\mathbf
  v}_{s} -\lambda \nabla^{2}T - \nabla_{i}\left(h\rho \frac{k_{ij}}{\mu} (\nabla_{j}P - \rho g_{j}) \right)
\end{equation}
Note that multiplication by fluid density $\rho$ occurs, irrespective of the `multiply_by_density` flag.

To represent the term $\frac{\partial}{\partial t}\left( (1 - \phi)\rho_{R}C_{R}T + \phi\rho\mathcal{E} \right)$ (which only appears in `transient` simulations) the `Action` adds a [PorousFlowEnergyTimeDerivative](PorousFlowEnergyTimeDerivative.md) Kernel.  This Kernel lumps the heat energy-density to the nodes.

To represent the term $\left( (1 - \phi)\rho_{R}C_{R}T + \phi\rho\mathcal{E} \right)\nabla\cdot{\mathbf v}_{s}$ (which only appears in `transient` simulations with `coupling_type = ThermoHydroMechanical`), the `Action` adds a [PorousFlowHeatVolumetricExpansion](PorousFlowHeatVolumetricExpansion.md) Kernel.

To represent the term $\lambda \nabla^{2}T$, the `Action` adds a [PorousFlowHeatConduction](PorousFlowHeatConduction.md) Kernel

To represent the term $\nabla_{i}\left(h\rho \frac{k_{ij}}{\mu} (\nabla_{j}P - \rho g_{j}) \right)$, the `Action` adds the following Kernels and possibly UserObjects, depending on the `stabilization` parameters:

- In the case of no-upwinding, a [PorousFlowFullySaturatedHeatAdvection](PorousFlowFullySaturatedHeatAdvection.md) Kernel with `multiply_by_density = true`.
- For full upwinding, a [PorousFlowFullySaturatedUpwindHeatAdvection](PorousFlowFullySaturatedUpwindHeatAdvection.md) Kernel.
- For [KT stabilization](kt.md), a [PorousFlowFluxLimitedTVDAdvection](PorousFlowFluxLimitedTVDAdvection.md) Kernel and a [PorousFlowAdvectiveFluxCalculatorSaturatedHeat](PorousFlowAdvectiveFluxCalculatorSaturatedHeat.md) UserObject with `multiply_by_density = true`.

## Solid-mechanics equations

The [static conservation of momentum equation](governing_equations.md) is associated with the displacement variables, and is only included if the `coupling_type` includes "Mechanical":
\begin{equation}
\label{eq:cons_mom}
0 = \nabla_{i}\sigma_{ij}^{\mathrm{eff}} - \alpha_{B}\nabla_{j} P - \rho_{\mathrm{undrained}} g_{j} \ .
\end{equation}
These equations are represented by a set of `StressDivergenceTensors` Kernels (or `StressDivergenceRZTensors` Kernels in the case of RZ coordinates), including thermal-eigenstrain coupling if `coupling_type = HydroThermoMechanical`, and `Gravity` Kernels.  These Kernels are added automatically and are part of the Tensor-Mechanics module.  The $\alpha_{B}\nabla_{j}P_{f}$ term is represented by a [PorousFlowEffectiveStressCoupling](PorousFlowEffectiveStressCoupling.md) Kernel.

It is assumed that the *effective stress not the total stress* enters into the
consitutive law (as above), and any plasticity, and any insitu stresses, and
almost everywhere else.  One exception is specifying Neumann boundary conditions
for the displacements where the total stresses are being specified, as can be
seen from [eq:cons_mom].  Therefore, MOOSE uses effective stress, and not total stress, internally.  If one needs to input or output total stress, one must subtract $\alpha_{B}P$ from MOOSE's stress.

## Sources and boundary conditions

Source terms may be added to the above equations by the user by including appropriate `Kernels` or `DiracKernels` (such as those listed [here](porous_flow/systems.md) and described [here](porous_flow/sinks.md)).  [Boundary conditions](porous_flow/boundaries.md) can also be suppled.  Sources and boundary conditions are not automatically included by the `Action`.

## Materials added

If required by the Kernels, the following Materials are added.  For each case, derivatives of the Material Properties are also created: these are with respect to mass fractions and porepressure, and, in the case of appropriate `coupling_type`, temperature and displacements.

[PorousFlow1PhaseFullySaturated](PorousFlow1PhaseFullySaturated.md) which creates `Material` versions of the porepressure and its spatial derivatives, saturation (which is always 1.0 in this case) and its spatial derivatives.

[PorousFlowTemperature](PorousFlowTemperature.md) which creates `Material` versions of the temperature and its spatial derivatives.

[PorousFlowMassFraction](PorousFlowMassFraction.md) which creates `Material` versions of the mass-fraction variables and their spatial derivatives

[PorousFlowSingleComponentFluid](PorousFlowSingleComponentFluid.md) or [PorousFlowBrine](PorousFlowBrine.md) or, in the case of `custom_fluid_properties`, a user-supplied Material to compute the fluid properties.  These properties are the fluid density and viscosity, as well as internal energy and enthalpy if required.

!alert note
Using non-standard `pressure_unit` or `time_unit` is only possible with `PorousFlowSingleComponentFluid`, not `PorousFlowBrine`.  You need to read the essay on these non-standard unit systems [here](PorousFlowSingleComponentFluid.md)

[PorousFlowEffectiveFluidPressure](PorousFlowEffectiveFluidPressure.md) which creates an effective fluid pressure, equal to the porepressure, for use in mechanically-coupled models.

[PorousFlowVolumetricStrain](PorousFlowVolumetricStrain.md) which creates a volumetric strain rate, for use in mechanically-coupled models.

[PorousFlowNearestQp](PorousFlowNearestQp.md) which records the nearest quadpoint to each node in each element.

!alert note
Even though there is only one fluid phase in this fully saturated action, some objects may require a relative permeability material to work. Examples include [PorousFlowDarcyVelocityComponent](PorousFlowDarcyVelocityComponent.md) `AuxKernels` which requires relative permeability, or [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md) wells. By default, this action adds a constant relative permeability of one, so that the fluid is perfectly mobile.

## Materials not added

Various important `Materials` are not added by this Action, so must be added by the user in the `[Materials]` block.  The reason these are not added by default is that they are usually subdomain-dependent.

- One of the [PorousFlowPorosity](/porous_flow/porosity.md) options
- One of the [PorousFlowPermeability](permeability.md) options
- [PorousFlowMatrixInternalEnergy](PorousFlowMatrixInternalEnergy.md)
- Thermal conductivity, such as [PorousFlowThermalConductivityIdeal](PorousFlowThermalConductivityIdeal.md)
- A definition of the elasticity tensor (eg [ComputeIsotropicElasticityTensor](/ComputeIsotropicElasticityTensor.md)) a strain calculator (eg [ComputeSmallStrain](/ComputeSmallStrain.md)) a thermal expansion eigenstrain calculator, (eg [ComputeThermalExpansionEigenstrain](/ComputeThermalExpansionEigenstrain.md)) and a stress calculator (eg [ComputeLinearElasticStress](/ComputeLinearElasticStress.md))
- Materials associated with chemistry, such as [PorousFlowAqueousPreDisChemistry](PorousFlowAqueousPreDisChemistry.md) and [PorousFlowAqueousPreDisMineral](PorousFlowAqueousPreDisMineral.md): see [tutorial 07](tutorial_07.md) for a worked example.

## AuxVariables

- If `add_darcy_aux = true` then the `Action` adds `constant monomial` auxillary variables with the names `darcy_vel_x`, `darcy_vel_y` and `darcy_vel_z` that record the Darcy velocity by using [PorousFlowDarcyVelocityComponent](PorousFlowDarcyVelocityComponent.md) AuxKernels.

- If `add_stress_aux = true` and the `coupling_type` includes "Mechanical", then the `Action` adds `constant monomial` auxillary variables with the name `stress_xx`, `stress_xy`, `stress_xz`, `stress_yx`, `stress_yy`, `stress_yz`, `stress_zx`, `stress_zy` and `stress_zz` that record the effective stress using a `RankTwoAux`.  As mentioned above, this is the effective stress: if you require total stress you need to subtract $\alpha_{B}P$, as in $\sigma_{ij}^{\mathrm{total}} = \sigma^{\mathrm{eff}}_{ij} - \delta_{ij}\alpha_{B}P$




!syntax parameters /PorousFlowFullySaturated/PorousFlowFullySaturated

!syntax inputs /PorousFlowFullySaturated/PorousFlowFullySaturated

!bibtex bibliography
