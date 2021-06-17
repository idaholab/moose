# PorousFlowBasicTHM

This Action allows simple simulation of fully-saturated, single-phase, single-component simulations.  It is mainly used in *poro-elastic* and *thermo-poro-elastic* simulations.  The fluid flow may be optionally coupled
to mechanics and/or heat flow using the `coupling_type` flag.

This Action offers simplified physics compared with the [PorousFlowFullySaturated](PorousFlowFullySaturated.md) and [PorousFlowUnsaturated](PorousFlowUnsaturated.md) Actions.

## Fluid flow

The fluid equation is a simplified form of the full [PorousFlow fluid equation](/porous_flow/governing_equations.md) (see [PorousFlowFullySaturatedMassTimeDerivative](/PorousFlowFullySaturatedMassTimeDerivative.md) and [PorousFlowFullySaturatedDarcyBase](/PorousFlowFullySaturatedDarcyBase.md) for the derivation):
\begin{equation}
\label{eq:basicthm}
0 = (\rho)\left(\frac{\dot{P}}{M} + \alpha_{B}\dot{\epsilon}_{v} - A\dot{T}\right) -
\nabla_{i}\left((\rho) k_{ij}\left(\nabla_{j}P - \rho g_{j}\right)/\mu\right)
\ .
\end{equation}
In this equation, the fluid density, $\rho$, appears in parentheses, because the user has the option of including it or not using the `multiply_by_density` flag.  Note that the fluid-mass time derivative is close to linear, and is perfectly linear if `multiply_by_density=false`, and this also almost linearises the flow term.  Extremely good nonlinear convergence should therefore be expected, but there are some knock-on effects that are documented in [PorousFlowFullySaturatedMassTimeDerivative](/PorousFlowFullySaturatedMassTimeDerivative.md).

The term $\alpha_{B}\dot{\epsilon}_{v}$ only appears if the `coupling_type` includes "Mechanical".  The term $A\dot{T}$ only appears if the `coupling_type` includes "Thermo".

To simulate [eq:basicthm] the `PorousFlowBasicTHM` Action employs the following Kernels:

- [PorousFlowFullySaturatedMassTimeDerivative](/PorousFlowFullySaturatedMassTimeDerivative.md), if the simulation is of Transient type.
- [PorousFlowFullySaturatedDarcyBase](/PorousFlowFullySaturatedDarcyBase.md)

For isothermal simulations, the fluid properties may still depend on temperature, so the `temperature` input parameter may be set to any real number, or to an AuxVariable if desired.

[Upwinding](/porous_flow/upwinding.md) and [mass lumping](/porous_flow/mass_lumping.md) are not used by these Kernels.  These numerical schemes are typically unnecessary in fully-saturated, single-phase simulations, but the lack of stabilization means the results from `PorousFlowBasicTHM` will typically be slightly different to the remainder of PorousFlow.

!alert note
Even though there is only one fluid phase in this fully saturated action, some objects may require a relative permeability material to work. Examples include [PorousFlowDarcyVelocityComponent](PorousFlowDarcyVelocityComponent.md) `AuxKernels` which requires relative permeability, or [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md) wells. By default, this action adds a constant relative permeability of one, so that the fluid is perfectly mobile. 

## Heat flow

For anisothermal simulations, the energy equation reads
\begin{equation}
0 = \frac{\partial}{\partial t}\left((1-\phi)\rho_{R}C_{R}T + \phi\rho\mathcal{E}\right) - \nabla\cdot (\rho h k(\nabla P - \rho \mathbf{g})/\mu) - \nabla\cdot(\lambda \nabla T) + \left((1-\phi)\rho_{R}C_{R}T + \phi\rho\mathcal{E}\right)\nabla\cdot v_{s} \ ,
\end{equation}
where the final term is only used if coupling with mechanics is also desired.  To simulate this DE, `PorousFlowBasicTHM` uses the following kernels:

- [PorousFlowEnergyTimeDerivative](/PorousFlowEnergyTimeDerivative.md) if the simulation is of Transient type
- [PorousFlowFullySaturatedHeatAdvection](/PorousFlowFullySaturatedHeatAdvection.md) with `multiply_by_density=true` (irrespective of the setting of this flag in the `PorousflowBasicTHM` Action)
- [PorousFlowHeatConduction](/PorousFlowHeatConduction.md)
- [PorousFlowHeatVolumetricExpansion](/PorousFlowHeatVolumetricExpansion.md) if coupling with temperature and mechanics is desired, and if the simulation is of Transient type

## Solid Mechanics

For simulations that couple fluid flow to mechanics, the equations are already written in [governing equations](/porous_flow/governing_equations.md), and `PorousFlowBasicTHM` implements these by using the following kernels:

- [StressDivergenceTensors](/StressDivergenceTensors.md)
- [Gravity](/Gravity.md)
- [PorousFlowEffectiveStressCoupling](/PorousFlowEffectiveStressCoupling.md)

## Required Materials

`PorousFlowBasicTHM` adds many Materials automatically, however, to run a simulation you will need to provide more Materials for each mesh block, depending on your simulation type, viz:

- One of the `PorousFlowPermeability` classes, eg [PorousFlowPermeabilityConst](PorousFlowPermeabilityConst.md)
- [PorousFlowConstantBiotModulus](/PorousFlowConstantBiotModulus.md)
- [PorousFlowConstantThermalExpansionCoefficient](/PorousFlowConstantThermalExpansionCoefficient.md)
- [PorousFlowPorosity](/PorousFlowPorosity.md)
- [PorousFlowMatrixInternalEnergy](/PorousFlowMatrixInternalEnergy.md)
- A thermal conductivity calculator, eg [PorousFlowThermalConductivityIdeal](/PorousFlowThermalConductivityIdeal.md)
- An elasticity tensor, eg, [ComputeIsotropicElasticityTensor](/ComputeIsotropicElasticityTensor.md)
- A strain calculator, eg, [ComputeSmallStrain](/ComputeSmallStrain.md)
- A thermal expansion eigenstrain calculator, eg, [ComputeThermalExpansionEigenstrain](/ComputeThermalExpansionEigenstrain.md)
- A stress calculator, eg [ComputeLinearElasticStress](/ComputeLinearElasticStress.md)

!alert note
Before choosing non-standard `pressure_unit` or `time_unit`, you should read the essay on these non-standard choices [here](PorousFlowSingleComponentFluid.md)


## Examples

A simple example of using `PorousFlowBasicTHM` is documented in the [PorousFlow tutorial](/porous_flow/tutorial_01.md) with input file:

!listing modules/porous_flow/examples/tutorial/01.i

A TH example that uses `PorousFlowBasicTHM` is also documented in the [PorousFlow tutorial](/porous_flow/tutorial_03.md) with input file:

!listing modules/porous_flow/examples/tutorial/03.i

A THM example that uses `PorousFlowBasicTHM` is also documented in the [PorousFlow tutorial](/porous_flow/tutorial_04.md) with input file:

!listing modules/porous_flow/examples/tutorial/04.i

!syntax parameters /PorousFlowBasicTHM/PorousFlowBasicTHM
