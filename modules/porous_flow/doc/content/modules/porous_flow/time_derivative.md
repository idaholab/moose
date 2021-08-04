# Numerical implementation of the time derivatives

## Introduction

The time derivatives in PorousFlow are usually [lumped to the nodes](porous_flow/mass_lumping.md).  In mechanically-coupled systems, there is an added complication of mesh deformation, and this page describes the particular implementation used in PorousFlow.  This page is self contained and uses notation that might be different from other PorousFlow documentation.

Consider a volume of porous material, $\Omega$.  $\Omega$ can be thought of as one finite element, or a region surrounding a node, but it could be any volume.  It is "attached" to the porous material (it is in the [Lagrangian reference frame](porous_flow/lagrangian_eulerian.md)): when the porous material moves or deforms, so does $\Omega$, relative to an external observer (in the [Eulerian reference frame](porous_flow/lagrangian_eulerian.md)) who is viewing the movement or deformation from outside the porous material.

The volume $\Omega$ contains a mass density of fluid, $M$ (measured in kg.m$^{-3}$, where the m$^{-3}$ is the volume of porous material).  In [PorousFlow](porous_flow/governing_equations.md), $M=\phi \rho S\chi$, but that is irrelevant here.  (The volume $\Omega$ could also contain a heat-energy density of fluid, and the same form of equation follows for it, so $M$ could also be thought of as heat-energy density.)  The total fluid mass within $\Omega$ is

\begin{equation}
  \mathrm{total}\ \mathrm{mass} = \int_{\Omega}M \ .
\end{equation}

Assume that there are sinks of fluid mass within $\Omega$, and denote these by $q$ (measured in kg.m$^{-3}$.s$^{-1}$).  There could also be boundary fluxes of fluid exiting or entering $\Omega$, but those can be handled in the same way as $q$, so can be ignored.  The fundamental equation of mass conservation is

\begin{equation}
  \label{eqn.fund.cons}
  0 = \frac{\mathrm{d}}{\mathrm{d} t} \int_{\Omega}M + \int_{\Omega}q
\end{equation}

## Simple worked example

In a small abuse of notation, denote the volume of the domain $\Omega$ by $\Omega$, that is $\int_{\Omega} = \Omega$.  For simplicity of presentation, consider the situation where $M=M(t)$, that is, $M$ does not vary spatially over the volume $\Omega$.  Remember that $\Omega$ can vary with $t$.  Then the first term of [eqn.fund.cons] may be written
\begin{equation}
  \frac{\mathrm{d}}{\mathrm{d} t} \int_{\Omega}M = \frac{\mathrm{d}}{\mathrm{d} t} \left(\Omega M\right)
\end{equation}
Consider two times, labelled "old" and "new", with time difference $\mathrm{d} t$.  Then
\begin{equation}
\begin{aligned}
  \frac{\mathrm{d}}{\mathrm{d} t} \int_{\Omega}M & = \frac{\Omega_{\mathrm{new}} M_{\mathrm{new}} - \Omega_{\mathrm{old}} M_{\mathrm{old}}}{\mathrm{d} t} \\
  & = \Omega_{\mathrm{old}} \frac{M_{\mathrm{new}} - M_{\mathrm{old}}}{\mathrm{d}t} + M_{\mathrm{new}} \frac{\Omega_{\mathrm{new}} - \Omega_{\mathrm{old}}}{\mathrm{d}t} \\
    & = \Omega_{\mathrm{old}} \left[ \frac{\partial M}{\partial t} + \frac{1}{\Omega_{\mathrm{old}}} M_{\mathrm{new}}\frac{\partial \Omega}{\partial t} \right] \ .
\end{aligned}
\end{equation}

## Connection with PorousFlow

To make the connection with PorousFlow clearer, assume that there is a displacement vector, $u_{i} = u_{i}(t)$, that defines the deformation $\Omega = \Omega(t)$.  Then the volume $\Omega(t) = \Omega_{0}(1 + \nabla\cdot u)$, where $\Omega_{0}$ is the volume in the undeformed material.  The volumetric strain is $\epsilon_{ii} = \nabla\cdot u$ (at least for small strains), and $\dot{\Omega} = \Omega_{0}\dot{\epsilon}_{ii}$.  Using this notation, substituting into [eqn.fund.cons] yields
\begin{equation}
  \label{eqn.fix}
  0 = \int_{\Omega_{0}}(1 + \nabla\cdot u_{\mathrm{old}}) \dot{M} + \int_{\Omega_{0}}M_{\mathrm{new}}\nabla\cdot\dot{u} + \int_{\Omega} q
\end{equation}
PorousFlow implements this as follows.

- The term $(1 + \nabla \cdot u_{\mathrm{old}})\dot{M}$ is [PorousFlowMassTimeDerivative](PorousFlowMassTimeDerivative.md) or [PorousFlowEnergyTimeDerivative](PorousFlowEnergyTimeDerivative.md).  Note that $(1 + \nabla\cdot u_{\mathrm{old}})$ is implemented in the code as $(1 + \epsilon_{ii}^{\mathrm{old}})$ which involves the total volumetric strain, $\epsilon_{ii}$ as calculated by a TensorMechanics strain calculator, for instance [ComputeSmallStrain](ComputeSmallStrain.md).  It is integrated over the undeformed mesh so should employ `use_displaced_mesh = false`.
- The term $M_{\mathrm{new}}\nabla\cdot\dot{u}$ is [PorousFlowMassVolumetricExpansion](PorousFlowMassVolumetricExpansion.md) or [PorousFlowHeatVolumetricExpansion](PorousFlowHeatVolumetricExpansion.md).  The $\nabla\cdot\dot{u}$ is implemented as $\dot{\epsilon}_{ii}$, which is the time-derivative of the total volumetric strain as calculated by a TensorMechanics strain calculator, and fed into the [PorousFlowVolumetricStrain](PorousFlowVolumetricStrain.md) Material.  It also should employ `use_displaced_mesh = false`.

## Mass and energy conservation

The [PorousFlowFluidMass](PorousFlowFluidMass.md) and [PorousFlowHeatEnergy](PorousFlowHeatEnergy.md) postprocessors use the total volumetric strain as calculated by [PorousFlowVolumetricStrain](PorousFlowVolumetricStrain.md).  Because the Kernels and the Postprocessors all use the same approach (assuming the `base_name` and `use_displaced_mesh` parameters are set correctly) mass and energy conservation is assured.

## Use of total strain

The PorousFlow implementation of $\Omega(t)$ is to use the `total_strain` as calculated by the TensorMechanics strain calculator, which in almost all PorousFlow models is [ComputeSmallStrain](ComputeSmallStrain.md).  Since the TensorMechanics strain calculators have a `base_name` input, users should take care to choose the appropriate `base_name` (without any typos) since PorousFlow cannot detect errors in this input.  PorousFlow simply checks whether a Material property called `base_name_total_strain` exists, and if so PorousFlow uses it, otherwise it continues without any total-strain contributions (corresponding to no mechanical coupling).

*By far the most common case is where `base_name` is not defined in the TensorMechanics strain calculator, so also does not need to be defined in PorousFlow.*  However, in more complicated situations, some potential use-cases and errors are:

- The simulation is mechanically coupled, but the user doesn't defined any TensorMechanics strain material.  Solution: define a strain material.
- The user desires a simulation to be mechanically coupled, but the user provides a `base_name` to PorousFlow that does not correspond to any TensorMechanics strain material.  In this case, PorousFlow simply thinks the user doesn't want mechanical coupling, and blindly continues.  Solution: ensure `base_name` is set to a TensorMechanics strain material.
- The simulation is mechanically coupled, but the user wants $\Omega$ to be defined separately from the TensorMechanics kinematics and constitutive law.  For instance, TensorMechanics might use finite strain, but PorousFlow might use small strains.  Solution: define more than one strain material, each with its own `base_name`, and provide PorousFlow with the appropriate `base_name`.
- The simulation is not mechanically coupled, but the user has a strain material in the input file that gets inadvertantly used by PorousFlow.  This is very difficult to debug, since PorousFlow does not warn it is using the strain material: PorousFlow simply thinks the user desires mechanical coupling.  Solution: provide something like `base_name = non_existent`: PorousFlow will detect there is no such strain calculator, and continue assuming no mechanical coupling.




