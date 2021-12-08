# CrystalPlasticityTwinningKalidindiUpdate

!syntax description /Materials/CrystalPlasticityTwinningKalidindiUpdate

`CrystalPlasticityTwinningKalidindiUpdate` is designed to be used in conjunction with the
[ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md) class to calculate
the response of a FCC crystalline solid. Details about the algorithm and specific
stress and strain measures used in the `CrystalPlasticityUpdate` base class are
given on the documentation page for
[ComputeMultipleCrystalPlasticityStress](/ComputeMultipleCrystalPlasticityStress.md).

Additionally `CrystalPlasticityTwinningKalidindiUpdate` is intended to be used with
a constitutive model for dislocation glide or slip: combining this twinning model
with [CrystalPlasticityKalidindiUpdate](/CrystalPlasticityKalidindiUpdate.md) will
replicate the coupling of the two constitutive models through the plastic velocity
gradient calcualtion as presented in [!cite](kalidindi2001).

## Constitutive Model Definition

Proposed by Kalidindi, this straight-forward twinning propagation model computes
the plastic shear increment due to twinning with a power law constiutive model:
\begin{equation}
  \Delta \gamma^{\beta} = \gamma_o \left| \frac{\tau^{\beta}}{g^{\beta}}  \right|^{1/m} \quad if \quad \tau^{\beta} > 0, \quad f_{total} < f_{limit}
\end{equation}
where $\gamma_o$ is the reference slip rate, $\tau^{\beta}$ is the applied resolved
shear stress, g$^{\beta}$ is the twin propagation resistance value, m is the slip
rate sensitivity exponent, f$_{total}$ is the sum of the twin volume fraction on
all twin systems, and f$_{limit}$ is the user-defined upper limit for the twin
volume fraction. This constitutive model does not allow de-twinning.

The rate of twin volume fraction on each twin system is calculated from the system
plastic shear increment
\begin{equation}
  \dot{f}^{\beta} = \frac{\dot{\gamma}^{\beta}}{\gamma_{tw}}
\end{equation}
where $\gamma_{tw}$ is the characteristic shear of the twin [!citep](kalidindi2001).

The twin propagation resistance is calculated as a function of the twin volume fraction
and the characteristic twin shear, with different hardening coefficients for
non-coplanar and coplanar twinning systems. The different coefficients are required
to capture experimental observations of early twinning deformation on non-coplanar
systems [!citep](kalidindi2001).
\begin{equation}
\Delta g^{\beta} = \gamma_{tw} \left[ h_{nc}\left( f_{total} \right)^b \sum_{nc}^k \dot{f}^k + h_{cp}\left( f_{total} \right) \sum_{cp}^k \dot{f}^k \right]
\end{equation}
where h$_{nc}$ is the user-defined non-coplanar hardening coefficent, b is the
hardening exponent for non-coplanar twin systems, and h$_{cp}$ is the user-defined
coplanar hardening coefficient.


### Plastic Velocity Gradient Calculation

In simulations where both twin propagation and dislocation slip constitutive models
are included, the influence of the twin propagation on the plastic velocity gradient
is implemented as a fraction of the twin volume fraction
\begin{equation}
\label{eqn:modTwinsLP}
L^P = \left(1 - {f_{total}}_{(n-1)} \right) \sum_{\alpha}^{slip} \dot{\gamma}^{\alpha} S^{\alpha}_o + \sum_{\beta}^{twin} \dot{f}^{\beta}\gamma_{tw}S^{\beta}_o
\end{equation}
where $\dot{\gamma}^{\alpha}$ is the plastic shear rate due to dislocation slip,
S$^{\alpha}_o$ is the Schmid tensor for the slip systems, and S$^{\beta}_o$ is the
Schmid tensor for the twinning system.
Note that the value of the total volume fraction of twins used in [eqn:modTwinsLP]
lags by single timestep. The use of this lagged value is designated by the $(n-1)$
subscript. This modification requires that the name for the total twin volume
fraction material property be provided to the dislocation slip constitutive model
as shown below.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/twinning/only_twinning_fcc.i block=Materials/twin_only_xtalpl

`CrystalPlasticityTwinningKalidindiUpdate` must be run in conjunction with the crystal
plasticity specific  stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/twinning/only_twinning_fcc.i block=Materials/stress

In most cases this twinning model is intended to be used with a dislocation glide
or slip model, such as [CrystalPlasticityKalidindiUpdate](/CrystalPlasticityKalidindiUpdate.md).
To couple the volume fraction of twins to the dislocation slip plastic velocity
gradient contribution, as shown in [eqn:modTwinsLP]. The name of the total twin
volume fraction material property must be supplied to the glide or slip material
model:

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/twinning/combined_twinning_slip_100compression.i block=Materials/slip_xtalpl

!syntax parameters /Materials/CrystalPlasticityTwinningKalidindiUpdate

!syntax inputs /Materials/CrystalPlasticityTwinningKalidindiUpdate

!syntax children /Materials/CrystalPlasticityTwinningKalidindiUpdate

!bibtex bibliography
