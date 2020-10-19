# Porosity

!syntax description /Materials/Porosity

## Description

`Porosity` computes the porosity, $f$, from the combined inelastic strain, $\epsilon_{in}$:
\begin{equation}
  f = (1.0 - f^{old}) * ({\epsilon}_{in} - {\epsilon}_{in}^{old}).\text{tr} + f^{old};
\end{equation}

Here, $\epsilon_{in}$ is typically computed by [ComputeMultipleInelasticStress](ComputeMultipleInelasticStress.md).

## Example Input Files

!listing modules/tensor_mechanics/test/tests/porosity/reg.i block=Materials

!syntax parameters /Materials/Porosity

!syntax inputs /Materials/Porosity

!syntax children /Materials/Porosity

!bibtex bibliography
