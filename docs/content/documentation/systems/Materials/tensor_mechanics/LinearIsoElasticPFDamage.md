# LinearIsoElasticPFDamage
!syntax description /Materials/LinearIsoElasticPFDamage

This part defines the positive part of strain energy and positive part of stress
that drive the crack propagation, and total stress is represented as a function
of damage parameter $c$, tensile part and compressive part of stress. The basic
idea and the procedures we follow is

1. EigenDecompose the strain or called spectral decomposition of strain tensor
2. Define the positive and negative part of strain energy due to tensile and
   compressive stress
3. Find the tensile/compressive stress which are the derivatives of positive/negative
   part of strain energy wrt strain tensor
4. Get the final stress which is a function of tensile compressive stress and
   damage parameter.

The following equations are used in this material:

$$
\begin{eqnarray}
  \psi = [(1-c)^2 + k]{\psi}^{+}_{0} - {\psi}^{-}_{0} \\
  {\psi}^{\pm}_{0} = \lambda \langle \epsilon_1 + \epsilon_2 + \epsilon_3 \rangle^{2}_{\pm} +\mu(\langle \epsilon_1 \rangle^{2}_{\pm} +\langle \epsilon_2 \rangle^{2}_{\pm} +\langle \epsilon_3 \rangle^{2}_{\pm}) \\
  {\sigma}^{\pm}_{0} = \frac{\partial {\psi}^{\pm}_{0}}{\partial \epsilon} = \sum \limits_{a=1}^3 [\lambda \langle \epsilon_1 + \epsilon_2 + \epsilon_3 \rangle_{\pm} + 2\mu \langle \epsilon_a \rangle_{\pm} ]n_a \otimes n_a \\
  \sigma = [(1-c)^2 + k]{\sigma}^{+}_{0} - {\sigma}^{-}_{0}
\end{eqnarray}
$$

Where
$$
\langle x \rangle_{\pm} = \frac{x \pm |x|}2
$$

The material provides the following material properties:

* `G0_pos` Positive part of strain energy
* `dG0_pos_dstrain` Tensile stress, which is the derivative of positive part of strain energy wrt strain tensor

!syntax parameters /Materials/LinearIsoElasticPFDamage

!syntax inputs /Materials/LinearIsoElasticPFDamage

!syntax children /Materials/LinearIsoElasticPFDamage
