# Finite Strain Crystal Plasticity

!syntax description /Materials/FiniteStrainCrystalPlasticity

## Description

!alert warning
The `FiniteStrainCrystalPlasticity` model is not actively developed.
Use of the [ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md)
system are recommend instead.

Constitutive models are used to calculate the plastic slip rate.
In this crystal plasticity material the slip rate is modeled as a power law:
\begin{equation}
  \label{eqn:powerLawSlipRate}
  \dot{\gamma}^{\alpha} = \dot{\gamma}_o \left| \frac{\tau^{\alpha}}{g^{\alpha}} \right|^{1/m} sign \left( \tau^{\alpha} \right)
\end{equation}
where $\dot{\gamma}_o$ is a reference slip rate, $\tau^{\alpha}$ is the applied
shear stress on each slip system $\alpha$, $g^{\alpha}$ is the slip system
strength, or resistance to slip, and $m$ is the strain rate sensitivity
exponent. The strength of each slip system is solved with an iterative process
as a function of the slip increment
\begin{equation}
  \label{eqn:slipStrengthEvolution}
  g^{\alpha} =  g_o + \Delta \gamma^{\alpha} q^{\alpha \beta} h _o \left| 1 - \frac{g^{\alpha}}{g_{sat}}  \right|^a sign \left( 1 - \frac{g^{\alpha}}{g_{sat}} \right)
\end{equation}
where $q^{\alpha \beta}$ is a hardening coefficient matrix that accounts for the
different in self and latent hardening, [eqn:selfLatentQMatrix], $h_o$
is an initial hardening term, $g_{sat}$ is a constant saturated hardening value,
and $a$ is the hardening exponent [!citep](kalidindi1992). The self and latent
hardening of the crystal is defined for an FCC system as
\begin{equation}
  \label{eqn:selfLatentQMatrix}
  q^{\alpha \beta} = \begin{Bmatrix}
                       1.0 & q   & q   & q  \\
                       q   & 1.0 & q   & q  \\
                       q   & q   & 1.0 & q  \\
                       q   & q   & q   & 1.0
                     \end{Bmatrix}
\end{equation}
where $q$ is a constant value of latent hardening among non-coplanar slip
systems. In [eqn:selfLatentQMatrix] the slip systems which share the
same slip plane normal (e.g. $[\bar{1}11]$) are coplanar and grouped together
with a latent hardening rate of unity [!citep](kalidindi1992). Each matrix entry in
[eqn:selfLatentQMatrix] represents the interaction among two different
coplanar slip system groups, that is a total of six slip systems
[!citep](kalidindi1992).

In this class, the  crystal strain and stress response with response to the
residual calculated by the second Piola-Kirchoff stress increment. In contrast,
in [FiniteStrainCPSlipRateRes](FiniteStrainCPSlipRateRes.md) the convergence of
the crystal plasticity strain and stress response is determined with respect to
the slip rate on each slip system of the crystal plasticity model.

## Units Assumed in the Crystal Plasticity Materials

The simulation domain for crystal plasticity models is resolved on the order of
individual crystal grains, and, as such, the mesh size is small. Although MOOSE
itself is dimension agnostic, the crystal plasticity models are implemented in
the +mm-MPa-s unit system+. This dimension system choice impacts the
input files in the following manner:

- Mesh dimensions should be constructed in mm
- Elastic constant values (e.g. Young's modulus and shear modulus) are entered in MPa
- Initial slip system strength values are entered in MPa
- Simulation times are given in s
- Strain rates and displacement loading rates are given in 1/s and mm/s, respectively

In physically based models, which maybe based on this class, initial densities
of crystal defects (e.g. dislocations, point defects) should be given in
1/mm$^2$ or 1/mm$^3$


!syntax parameters /Materials/FiniteStrainCrystalPlasticity

!syntax inputs /Materials/FiniteStrainCrystalPlasticity

!syntax children /Materials/FiniteStrainCrystalPlasticity

!bibtex bibliography
