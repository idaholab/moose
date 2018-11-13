# GrandPotentialInterface

!syntax description /Materials/GrandPotentialInterface

The multiphase Grand Potential model is parameterized using a bulk free energy
coefficient $\mu$, a gradient interface coefficient $\kappa$, and a set of
interface pair coefficients $\gamma_{ij}$.

This material class provides the above mentioned parameters and calculates them
using the physical parameters of the free energy area density $\sigma_{ij}$ (`sigma`) for
the interface between each pair of phases, and an interface width $l$ (`width`).

To compute the parameters first either the median of all $\sigma_{ij}$ is chosen
or, if supplied by the user, the $\sigma_{ij}$ entry with the index
`sigma_index` is chosen (overriding the median computation).  The chosen
$\sigma_{ij}$ is assigned a value $\gamma_{ij}=1.5$. For this gamma value a set
of analytical expessions holds

\begin{equation}
\begin{aligned}
\kappa &= \frac 34 \sigma_{ij} l_{ij}\\
\mu &= \frac{6 \sigma_{ij}} {l_{ij}}.
\end{aligned}
\end{equation}

With $\kappa$ and $\mu$ determined the remaining $\gamma_{ij}$ can be computed
using the fitted relation

\begin{equation}
\begin{aligned}
g_{ij} &= \frac{\sigma_{ij}}{\sqrt{\mu\kappa}}  \\
\gamma_{ij} &= \left( -5.288 g_{ij}^8 -0.09364 g_{ij}^6 + 9.965 g_{ij}^4 -8.183 g_{ij}^2 + 2.007 \right)^{-1}.
\end{aligned}
\end{equation}

The material propertied provided by this class are directly used by the
[`ACGrGrMulti`](/ACGrGrMulti.md) and [`ACInterface`](/ACInterface.md) objects
and indirectly used by the
[GrandPotentialKernelAction](/action/GrandPotentialKernelAction.md).

!syntax parameters /Materials/GrandPotentialInterface

!syntax inputs /Materials/GrandPotentialInterface

!syntax children /Materials/GrandPotentialInterface

!bibtex bibliography
