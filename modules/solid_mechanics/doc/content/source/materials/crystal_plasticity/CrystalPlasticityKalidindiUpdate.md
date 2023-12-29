# CrystalPlasticityKalidindiUpdate

!syntax description /Materials/CrystalPlasticityKalidindiUpdate

`CrystalPlasticityKalidindiUpdate` is designed to be used in conjunction with the
[ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) class to calculate
the response of a FCC crystalline solid. Details about the algorithm and specific
stress and strain measures used in the `CrystalPlasticityUpdate` base class are
given on the documentation page for
[ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md).

## Constitutive Model Definition

The self and latent hardening crystal plasticity model developed by
[!cite](kalidindi1992) is implemented in this class. Here the slip rate is given
as a power law relationship:
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
and $a$ is the hardening exponent [!citep](kalidindi1992).

The self and latent hardening for the assumed FCC system is given as
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
coplanar slip system groups, for a total of six slip systems
[!citep](kalidindi1992).


### Connection to Twinning Constitutive Models

In simulations where both twin propagation and dislocation slip constitutive models
are included, the influence of the twin propagation on the plastic velocity gradient
is implemented as a fraction of the total twin volume fraction [!citep](kalidindi2001)
\begin{equation}
L^P = \left(1 - {f_{total}}_{(n-1)} \right) \sum_{\alpha}^{slip} \dot{\gamma}^{\alpha} S^{\alpha}_o + \sum_{\beta}^{twin} \dot{f}^{\beta}\gamma_{tw}S^{\beta}_o
\end{equation}
where f$_{total}$ is the sum of the twin volume fraction on all twin systems,
$\dot{\gamma}^{\alpha}$ is the plastic shear rate due to dislocation slip,
S$^{\alpha}_o$ is the Schmid tensor for the slip systems, and S$^{\beta}_o$ is the
Schmid tensor for the twinning system.
Note that the value of the total volume fraction of twins lags by single timestep.
The use of this lagged value is designated by the $(n-1)$ subscript.
This modification requires that the name for the total twin volume
fraction material property be provided to the dislocation slip constitutive model
as shown below.


## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/trial_xtalpl

`CrystalPlasticityKalidindiUpdate` must be run in conjunction with the crystal
plasticity specific  stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/stress_update_material_based/update_method_test.i block=Materials/stress

In simulations which couple this dislocation slip constitutive model with a twin
propagation model, such as
[CrystalPlasticityTwinningKalidindiUpdate](/CrystalPlasticityTwinningKalidindiUpdate.md),
the name of the total twin volume fraction material property must be supplied
in the input file:

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/twinning/combined_twinning_slip_100compression.i block=Materials/slip_xtalpl

!syntax parameters /Materials/CrystalPlasticityKalidindiUpdate

!syntax inputs /Materials/CrystalPlasticityKalidindiUpdate

!syntax children /Materials/CrystalPlasticityKalidindiUpdate

!bibtex bibliography
