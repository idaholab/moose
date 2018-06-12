# Power Law Softening

!syntax description /Materials/PowerLawSoftening

## Description

The material `PowerLawSoftening` computes the reduced stress and stiffness along
the direction of a crack according to a power law equation. The computed
reduced stiffness softens the tensile response of the material once the principle
stress applied to a material exceeds the cracking stress threshold of the material.

As with the other smeared cracking softening models, which all follow the
nomenclature convention of using the `Softening` suffix, this model is intended
to be used with the [ComputeSmearedCrackingStress](/ComputeSmearedCrackingStress.md)
material.

### Softening Model

The tensile stress response to cracking is calculated as a function of the number
cracks, where the number of cracks reduces the stress reponse of the cracked material.
The calculated stress is the principle stress in the single direction of the crack.
\begin{equation}
  \label{eqn:power_law_softening_stress}
  \sigma = k E \epsilon_{principle}
\end{equation}
where $k$ is the reduction factor applied to the initial stiffness each time a
new crack initiates, $E$ is the Youngs' modulus, and $\epsilon_{principle}$ is the
strain along the direction of the crack.
The reduction factor in [eqn:power_law_softening_stress] is a function of the
number of cracks
\begin{equation}
  \label{eqn:reduction_factor}
  k = k_0 \left( k_r\right)^n
\end{equation}
where $k_0$ is the initial cracking reduction factor and $n$ is the number of cracks.
The form of [eqn:reduction_factor] gives the `PowerLawSoftening` model its name.

In the context of the smeared cracking modeling approach, individual cracks are
not tracked; therefore, [eqn:reduction_factor] is approximated by a single
constant input parameter.
The user should consult additional resources to determine a reasonable value for
the initial stiffness reduction factor.


## Example Input File

!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking_power.i block=Materials/power_law_softening

`PowerLawSoftening` must be run in conjunction with the fixed smeared cracking material model as shown below:

!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking_power.i block=Materials/elastic_stress

!syntax parameters /Materials/PowerLawSoftening

!syntax inputs /Materials/PowerLawSoftening

!syntax children /Materials/PowerLawSoftening

!bibtex bibliography
