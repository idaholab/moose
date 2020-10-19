# ADPorosity

!syntax description /Materials/ADPorosity

## Description

`ADPorosity` computes the porosity, $f$, from the combined inelastic strain, $\epsilon_{in}$:
\begin{equation}
  f = (1.0 - f^{old}) * ({\epsilon}_{in} - {\epsilon}_{in}^{old}).\text{tr} + f^{old};
\end{equation}

Here, $\epsilon_{in}$ is typically computed by [ADComputeMultipleInelasticStress](ADComputeMultipleInelasticStress.md) using [ADViscoplasticityStressUpdate](ADViscoplasticityStressUpdate.md) methods.

## Example Input Files

!listing modules/tensor_mechanics/test/tests/porosity/d.i block=Materials

!syntax parameters /Materials/ADPorosity

!syntax inputs /Materials/ADPorosity

!syntax children /Materials/ADPorosity

!bibtex bibliography
