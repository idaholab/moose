# ADKKSPhaseChemicalPotential

!syntax description /Kernels/ADKKSPhaseChemicalPotential

## Description

Enforces the point wise
equality of the phase chemical potentials

\begin{equation}
\frac{dF_a}{dc_a}=\frac{dF_b}{dc_b}.
\end{equation}

The non-linear variable of this Kernel is $c_a$.

### Residual

\begin{equation}
R=\frac{dF_a}{dc_a} - \frac{dF_b}{dc_b}
\end{equation}

!syntax parameters /Kernels/ADKKSPhaseChemicalPotential

!syntax inputs /Kernels/ADKKSPhaseChemicalPotential

!syntax children /Kernels/ADKKSPhaseChemicalPotential
