# Crystal Plasticity HCP Dislocation Slip Beyerlein Update

!syntax description /Materials/CrystalPlasticityHCPDislocationSlipBeyerleinUpdate

## Description

`CrystalPlasticityHCPDislocationSlipBeyerleinUpdate` is designed to be used in conjunction with the
[ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) class to calculate
the response of a FCC crystalline solid. Details about the algorithm and specific
stress and strain measures used in the `CrystalPlasticityUpdate` base class are
given on the documentation page for
[ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md).

As in other crystal plasticity consitutive models compatible with [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md), the user must supply the slip plane normal and slip direction information, and the lengths of the unit cell lattice parameters.
Unlike other crystal plasticity consistutive models, `CrystalPlasticityHCPDislocationSlipBeyerleinUpdate` is set for use with the HCP lattice type only. Because of this lattice type requirement, four entries each are expected for both the slip plane normal and the slip direction vector entries.

## Constitutive Model Definition

A constitutive model for only the glide and evolution of forest dislocations within a Hexagonal Close-Packed (HCP) crystal lattice is implemented in this class. The constitutive model is taken from the work of [!cite](beyerlein2008dislocation,capolungo2009interaction,beyerlein2010probabilistic), which was developed for a viscoplastic self-consistent application. Here the constitutive model is employed within a crystal plasticity implementation.

`CrystalPlasticityHCPDislocationSlipBeyerleinUpdate` allows for the evaluation of the slip resistance, forest dislocation density, and substructure density on different types of slip systems, such as basal$<a>$, prismatic$<a>$, pyramidal$<c+a>$, and so on. Within this class these different types of slip systems are termed `modes`, following [!cite](beyerlein2008dislocation). Vectors of values for each slip mode dependent parameter may be supplied within the input file. The number of slip systems, value of the burgers vector, and the value of the initial lattice friction are examples of slip mode dependent input file parameters.
The specification of modes are not explicityly identified within the code, and no particular order is enforced.

!alert warning
Users are responsible for ensuring that the order of mode-specific parameters is consistent throughout the input file, and that the order of slip system modes corresponds correctly to the order of slip plane normals and slip directions given in the slip system text file, `slip_sys_file_name`.

### Slip System Resistance Calculation

The resistance to dislocation glide is represented in this constitutive model as the strength value that the applied shear stress must overcome to produce plastic slip. The slip resistance is calculated individually for each of the user-supplied slip systems. The resistance is considered here as the sum of four terms:
\begin{equation}
  \label{eqn:hcpGlideTotalSlipResistance}
  g^{\alpha} = g^{\alpha}_o + g^{\alpha}_{HP} + g^{\alpha}_{forest} + g^{\alpha}_{sub}
\end{equation}
where g$^{\alpha}_o$ is a user-defined constant initial lattice friction, g$^{\alpha}_{HP}$ represents a Hall-Petch type treatment of the slip resistance dependence on grain size, and g$^{\alpha}_{forest}$ and g$^{\alpha}_{sub}$ represent the hardening contributions from the two dislocation density populations, forest and substructure. [!citep](beyerlein2008dislocation).The grain-size dependence hardening term employs a Hall-Petch type dependence on grain size, following [!cite](beyerlein2008dislocation):
\begin{equation}
  \label{eqn:hallPetchTypeSlipResistance}
  g^{\alpha}_{HP} = HP^{\alpha}\mu^{\alpha} \sqrt{\frac{b^{\alpha}}{d_g}}
\end{equation}
where HP$^{\alpha}$ is a user-defined constant, $\mu^{\alpha}$ is the shear modulus for the slip system, b$^{\alpha}$ is the slip system Burgers vector, and d$_g$ is the average grain size. This value is computed once during the initalization stages of the simulation.

The hardening contribution from forest dislocations, see [forest dislocation evolution ](#forest_dislocation_evolution) discussion, is calculated on a per slip basis[!citep](capolungo2009interaction).
\begin{equation}
  \label{eqn:forestDislocationSlipResistance}
  g^{\alpha}_{forest} = \chi b^{\alpha} \mu^{\alpha} \sqrt{\rho^{\alpha}_{forest}}
\end{equation}
where $\chi$ is a user-defined interaction coefficient and $\rho^{\alpha}_{forest}$ is the density of forest dislocations on the slip system ${\alpha}$. This calculation is completed at every timestep to capture the dislocation density evolution.

Similarly, the hardening contribution due to the substructure density is computed once per timestep; however, only a single total value of substructure density from all slip systems is tracked, as described in the [substructure evolution](#substructure_evolution) section. Characteristics of the slip systems, such as the Burgers vector, do affect the hardening contribution from substructure density on each slip system:
\begin{equation}
  \label{eqn:substructureSlipResistance}
  g^{\alpha}_{sub} = k_{sub} b^{\alpha} \mu^{\alpha} \rho_{sub} \text{log}\left(\frac{1}{b^{\alpha}\sqrt{\rho_{sub}}} \right)
\end{equation}
where k$_{sub}$ is a user-defined interaction coefficient and $\rho_{sub}$ is the substructure density.

### Slip Rate Calculation

The slip rate due to dislocation glide is calculated with a power law relation:
\begin{equation}
  \label{eqn:powerLawSlipRateModel}
  \Delta \gamma^{\alpha} = \left( \dot{\gamma}_o \left| \frac{\tau^{\alpha}}{g^{\alpha}} \right|^{1/m} sign \left(\tau^{\alpha}\right) \right) \Delta t
\end{equation}
where $\dot{\gamma}_o$ is the reference strain rate, $\tau^{\alpha}$ is the resolved applied shear stress on the slip system $\alpha$, m is the slip rate exponent, and $\Delta$t is the current timestep size.
As in the work of [!cite](beyerlein2008dislocation) a low slip rate exponent is recommended here to produce slip only when the applied shear stress is near the value fo the slip system resistance, [eqn:hcpGlideTotalSlipResistance]. Note that lower values of the strain rate exponent can cause strain rate sensitivity; following [!cite](capolungo2009interaction) and [!cite](beyerlein2010probabilistic), `CrystalPlasticityHCPDislocationSlipBeyerleinUpdate` sets the reference strain rate, $\dot{\gamma}_o$ equal to the macroscopic strain rate through a user-supplied parameter.
To reduce potential issues arising from very low slip increments, the requirement
\begin{equation}
  \frac{\tau^{\alpha}}{g^{\alpha}} > \text{tolerance}
\end{equation}
is implemented in this class, where the tolerance is a user-defined parameter. For slip systems which do not meet this threshold requirement, the slip increment value is set to zero for that timestep.

### Forest Dislocation Evolution id=forest_dislocation_evolution

The forest dislocations evolve in response to the resolved applied shear stress, which provides the driving force for dicloscation glide. The forest dislocation density evolution model is a direct function of the slip increment [!citep](beyerlein2008dislocation):
\begin{equation}
  \Delta \rho^{\alpha}_{forest} = k_1 \sqrt{\rho^{\alpha}_{forest}} \left| \Delta \gamma^{\alpha} \right| - \Delta \rho^{\alpha}_{rem}
\end{equation}
where k$_1$ is the slip mode dependent forest dislocation generation coefficient and $\rho^{\alpha}_{rem}$ is the removed forest dislocation density increment for the current timestep. These forest dislocations are either annihilated or generated substructure debris. The evolution of removed forest dislocations has the form [!citep](beyerlein2008dislocation):
\begin{equation}
  \label{eqn:removedForestDislocations}
  \Delta \rho^{\alpha}_{rem} = \frac{\chi b^{\alpha}}{G^{\alpha}}k_1 \left[ 1 - \frac{kT}{D^{\alpha}\left(b^{\alpha}\right)^3}  ln \left(\frac{\dot{\epsilon}}{\epsilon_0} \right)\right] \left| \Delta \gamma^{\alpha} \right|
\end{equation}
where G$^{\alpha}$ is a slip-mode specific normalized activation energy, k is the Boltzman constant, T is the temperature, D$^{\alpha}$ is the slip mode dependent propotionality factor, $\dot{\epsilon}$ is the applied strain rate, bounded within the range 10$^{-5}$ to 10$^{-3}$ 1/s, and $\epsilon_0$ is a material value estimated to be 10$^7$ [!citep](beyerlein2008dislocation).

The final forest dislocation density per slip system is updated at the end of each time step as
\begin{equation}
  \rho^{\alpha}_{forest}(n) = \rho^{\alpha}_{forest}(n-1) + \Delta\rho^{\alpha}_{forest}
\end{equation}
where n is the current timestep and n-1 is the previous timestep, provided the increment of the forest dislocation density is within the user-set tolerance. If the forest density increment is out of tolerance, `CrystalPlasticityHCPDislocationSlipBeyerleinUpdate` triggers the use of a smaller timestep or substep.


### Substructure Dislocation Evolution id=substructure_evolution

The substructure density is calculated as a function of the removed forest dislocation density, [eqn:removedForestDislocations], following [!cite](capolungo2009interaction):
\begin{equation}
  \label{eqn:substructureDensity}
  \Delta \rho_{sub} = \sum_{\alpha}f^{\alpha}b^{\alpha}\Delta \rho^{\alpha}_{rem} \sqrt{\rho_{sub}}
\end{equation}
where f$^{\alpha}$ is the slip-mode dependent substructure generation rate coefficient. The substructure density is considered to affect the forest dislocation evolution on all slip systesms eqvenly; thus, only the total sum of substructure density across all slip systems is modeled here. As noted by [!citep](capolungo2009interaction), the constitutive model does nto allow for recovery of dislocations from the substructure density.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/hcp_single_crystal/update_method_hcp_aprismatic_capyramidal.i block=Materials/trial_xtalpl

`CrystalPlasticityHCPDislocationSlipBeyerleinUpdate` must be run in conjunction with the crystal
plasticity specific  stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/hcp_single_crystal/update_method_hcp_aprismatic_capyramidal.i block=Materials/stress

!syntax parameters /Materials/CrystalPlasticityHCPDislocationSlipBeyerleinUpdate

!syntax inputs /Materials/CrystalPlasticityHCPDislocationSlipBeyerleinUpdate

!syntax children /Materials/CrystalPlasticityHCPDislocationSlipBeyerleinUpdate

!bibtex bibliography
