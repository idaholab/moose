# Exponential Softening

!syntax description /Materials/ExponentialSoftening

## Description

The material `ExponentialSoftening` computes the reduced stress and stiffness
in the direction of a crack according to a exponential function. The computed
cracked stiffness ratio softens the tensile response of the material once the
principle stress exceeds the cracking stress threshold of the material.

As with the other smeared cracking softening models, which all follow the
nomenclature convention of using the `Softening` suffix, this model is intended
to be used with the [ComputeSmearedCrackingStress](/ComputeSmearedCrackingStress.md)
material.

### Softening Model

The tensile stress response to cracking is calculated as an exponential function
of the crack strain
\begin{equation}
  \label{eqn:exp_crack_stress}
  \sigma = \sigma_c \cdot \left( \sigma_{res} + (1 - \sigma_{res}) \cdot
       \exp \left[ \frac{\alpha \beta}{\sigma_c} \cdot \left( \epsilon_c^{max}
       - \epsilon_c^{init} \right) \right] \right)
\end{equation}
where the calculated stress, $\sigma$ is the principle stress along the direction
of the crack, $\sigma_c$ is the stress threshold beyond which cracking occurs,
$\sigma_{res}$ is the residual stress retained after full softening due to the
crack is completed, $\alpha$ is the initial slope of the exponential curve,
$\beta$ is a fitting parameter, $\epsilon_c^{max}$ is the maximum strain in the
direction of crack, and $\epsilon_c^{init}$ is the strain in direction of crack
when crack initiation occurred.
The ratio of the current stiffness to the original material stiffness is
computed using the result of [eqn:exp_crack_stress]
\begin{equation}
  \label{eqn:exp_stiffness_ratio}
  R = \sigma \cdot \frac{\epsilon_c^{init}}{\epsilon_c^{max}\sigma_c}
\end{equation}
where the definitions for the variables are the same here as in
[eqn:exp_crack_stress]. The stiffness ratio is passed back to the
[ComputeSmearedCrackingStress](/ComputeSmearedCrackingStress.md)
to compute the softened cracked material stiffness.


## Example Input File

!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking_rotation.i block=Materials/exponential_softening

`ExponentialSoftening` must be run in conjunction with the fixed smeared cracking material model as shown below:

!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking_rotation.i block=Materials/cracking_stress

!syntax parameters /Materials/ExponentialSoftening

!syntax inputs /Materials/ExponentialSoftening

!syntax children /Materials/ExponentialSoftening

!bibtex bibliography
