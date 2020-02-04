# MixedModeEquivalentK

!syntax description /VectorPostprocessors/MixedModeEquivalentK

## Description

This class computes an equivalent stress intensity factor $K_{eq}$ for a crack under mixed-mode loading as a function of the stress intensity factors for the individual modes ($K_{I}$, $K_{II}$, and $K_{III}$ for modes $I$, $II$, and $III$, respectively):
\begin{equation}
K_{eq}=\sqrt{K_I^2 + K_{II}^2 + \frac{1}{1-\nu}K_{III}^2}
\end{equation}
where $\nu$ is the Poisson's ratio. This object is a VectorPostprocessor, and the individual stress intensity factors are computed by individual VectorPostprocessors.

This object is not typically defined directly by a user, but is set up automatically using the `equivalent_K` parameter in [DomainIntegralAction](/DomainIntegralAction.md).

!syntax parameters /VectorPostprocessors/MixedModeEquivalentK

!syntax inputs /VectorPostprocessors/MixedModeEquivalentK

!syntax children /VectorPostprocessors/MixedModeEquivalentK
