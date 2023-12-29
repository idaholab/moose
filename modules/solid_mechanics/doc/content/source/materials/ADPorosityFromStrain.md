# ADPorosityFromStrain

!syntax description /Materials/ADPorosityFromStrain

## Description

`ADPorosityFromStrain` computes the porosity, $f$, from the combined inelastic strain, $\epsilon_{in}$:
\begin{equation}
  f = (1.0 - f^{old}) * ({\epsilon}_{in} - {\epsilon}_{in}^{old}).\text{tr} + f^{old};
\end{equation}

Here, $\epsilon_{in}$ is typically computed by [ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md) using [ADViscoplasticityStressUpdate](ADViscoplasticityStressUpdate.md) methods.

## Example Input Files

!listing modules/tensor_mechanics/test/tests/porosity/ad.i block=Materials

!syntax parameters /Materials/ADPorosityFromStrain

!syntax inputs /Materials/ADPorosityFromStrain

!syntax children /Materials/ADPorosityFromStrain

!bibtex bibliography
