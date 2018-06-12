# Abrupt Softening

!syntax description /Materials/AbruptSoftening

## Description

The material `AbruptSoftening` computes the reduced stress and stiffness
in the direction of a crack according to a step function. The computed
cracked stiffness ratio softens the tensile response of the material once the
principle stress exceeds the cracking stress threshold of the material.

As with the other smeared cracking softening models, which all follow the
nomenclature convention of using the `Softening` suffix, this model is intended
to be used with the [ComputeSmearedCrackingStress](/ComputeSmearedCrackingStress.md)
material.

### Softening Model

As the class name implies, `AbruptSoftening` does not allow any gradual softening
of the material and instantly drops the stiffness of the material in response to
cracking.
The tensile stress response to cracking is based on the value of the residual
stress retained after softening, $\sigma_{res}$, and is given as
\begin{equation}
  \label{eqn:abrupt_crack_stress}
  \sigma = \begin{cases}
            1 \times 10^{-16} \cdot E \epsilon_c^{init} & \text{ if } \sigma_{res} = 0 \\
            \sigma_{res} \cdot \sigma_c & \text{ if } \sigma_{res} \neq 0
           \end{cases}
\end{equation}
where the calculated stress, $\sigma$ is the principle stress along the direction
of the crack, $\sigma_c$ is the stress threshold beyond which cracking occurs,
$E$ is the Youngs' modulus value, and $\epsilon_c^{init}$ is the strain in
direction of the crack when crack initiation occurred.
The ratio of the current stiffness to the original material stiffness is
similiarly determined based on the value of the residual stress
\begin{equation}
  \label{eqn:abrupt_stiffness_ratio}
  R = \begin{cases}
        1 \times 10^{-16} & \text{ if } \sigma_{res} = 0 \\
        \frac{\sigma}{E \epsilon_c^{max}} & \text{ if } \sigma_{res} \neq 0
       \end{cases}
\end{equation}
where $\sigma$ is the principle stress along the direction of the crack,
$E$ is the Youngs' modulus value, and $\epsilon_c^{max}$ is the maximum strain
in the direction of crack.
The stiffness ratio is passed back to the
[ComputeSmearedCrackingStress](/ComputeSmearedCrackingStress.md)
to compute the softened cracked material stiffness.


## Example Input File

!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking_rz.i block=Materials/abrupt_softening

`AbruptSoftening` must be run in conjunction with the fixed smeared cracking material model as shown below:

!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking_rz.i block=Materials/elastic_stress

!syntax parameters /Materials/AbruptSoftening

!syntax inputs /Materials/AbruptSoftening

!syntax children /Materials/AbruptSoftening

!bibtex bibliography
