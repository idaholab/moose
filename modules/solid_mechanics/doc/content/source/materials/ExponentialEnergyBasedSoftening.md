# Exponential Energy Based Softening

!syntax description /Materials/ExponentialEnergyBasedSoftening

## Description

The material `ExponentialEnergyBasedSoftening` computes the reduced stress and stiffness
in the direction of a crack according to a exponential function. The computed
cracked stiffness ratio softens the tensile response of the material once the
principal stress exceeds the cracking stress threshold of the material.

As with the other smeared cracking softening models, which all follow the
nomenclature convention of using the `Softening` suffix, this model is intended
to be used with the [ComputeSmearedCrackingStress](/ComputeSmearedCrackingStress.md)
material.

### Softening Model

This model calculates the tensile stress response in the same way as the
[ExponentialSoftening](/ExponentialSoftening.md) model, except that the initial softening 
rate $\alpha\beta$ is calculated based on the fracture toughness and element size.

The linear element size, $l$, is estimated from the volume or area, $v$, depending on the dimension
of the problem. For three dimensional elements, $l = v^{1/3}$, and for two dimensional elements, $l = v^{1/2}$.

Then, the initial slope is calculated as
\begin{equation}
  \label{eqn:kic_exp_slope}
  \alpha\beta = \frac{\sigma_c^2}{G/l - \sigma_c^2/(2*E)}
\end{equation}
where G is the energy release rate, $\sigma_c$ is the tensile strength, and $E$ is the elastic modulus. $G$ is related to the fracture toughness, $K_{Ic}$ as expressed below.
\begin{equation}
  \label{eqn:energyreleaserate}
  G = \frac{K_{Ic}^2 * (1 - \nu^2)}{E}
\end{equation}
where $\nu$ is the Poisson's ratio.

This element size-dependent calculation of the initial slope is applicable until the element size reaches a maximum, $l_{max} = 2GE/\sigma_c^2$. Once the element size $l$ is larger than $l_{max}$, the initial softening slope 
is set to $\alpha\beta = -1e5 * E$, essentially approximating an abrupt softening behavior. The $-1e5$ multiplier on $E$ in this equation chosen because it is a large negative number that results in a large negative slope appropriate for the model units.

!syntax parameters /Materials/ExponentialEnergyBasedSoftening

!syntax inputs /Materials/ExponentialEnergyBasedSoftening

!syntax children /Materials/ExponentialEnergyBasedSoftening

!bibtex bibliography
