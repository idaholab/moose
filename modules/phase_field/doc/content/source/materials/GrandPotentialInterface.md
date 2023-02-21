# GrandPotentialInterface

!syntax description /Materials/GrandPotentialInterface

The multiphase Grand Potential model is parameterized using a bulk free energy
coefficient $\mu$, a gradient interface coefficient $\kappa$, and a set of
interface pair coefficients $\gamma_{\alpha i \beta j}$ [!cite](AagesenGP2018).
Note that this model is a multi phase / poly crystal model and the indices
$\alpha$ and $\beta$ represent phases and $i$ and $j$ represent grains.

This material class provides the above mentioned parameters and calculates them
using the physical parameters of the free energy area density $\sigma_{\alpha i
\beta j}$ (`sigma`) for the interface between each pair of phases, and an
interface width $l$ (`width`).

To compute the parameters first either the median of all $\sigma_{\alpha i \beta
j}$ is chosen or, if supplied by the user, the $\sigma_{\alpha i \beta j}$ entry
with the index `sigma_index` is chosen (overriding the median computation).  The
chosen $\sigma_{\alpha i \beta j}$ is assigned a value $\gamma_{\alpha i \beta
j}=1.5$. For this gamma value a set of analytical expessions holds

\begin{equation}
\begin{aligned}
\kappa &= \frac 34 \sigma_{\alpha i \beta j} l_{\alpha i \beta j}\\
\mu &= \frac{6 \sigma_{\alpha i \beta j}} {l_{\alpha i \beta j}}.
\end{aligned}
\end{equation}

!alert note title=Interface widths
Note that the interface with $l$ (`width`) is only guaranteed for the interface
with either the median $\sigma_{\alpha i \beta j}$ or - if provided - the index
supplied in `sigma_index`. All other interface widths are a function of their
respective interfacial free energies.

With $\kappa$ and $\mu$ determined the remaining $\gamma_{\alpha i \beta j}$ can
be computed using the fitted relation [!cite](MoelansWeb)

\begin{equation}
\begin{aligned}
g_{\alpha i \beta j} &= \frac{\sigma_{\alpha i \beta j}}{\sqrt{\mu\kappa}}  \\
\gamma_{\alpha i \beta j} &= \left( -5.288 g_{\alpha i \beta j}^8 -0.09364 g_{\alpha i \beta j}^6 + 9.965 g_{\alpha i \beta j}^4 -8.183 g_{\alpha i \beta j}^2 + 2.007 \right)^{-1}.
\end{aligned}
\end{equation}

The material propertied provided by this class are directly used by the
[`ACGrGrMulti`](/ACGrGrMulti.md) and [`ACInterface`](/ACInterface.md) objects
and indirectly used by the
[GrandPotentialKernelAction](/actions/GrandPotentialKernelAction.md).

!syntax parameters /Materials/GrandPotentialInterface

!syntax inputs /Materials/GrandPotentialInterface

!syntax children /Materials/GrandPotentialInterface

!bibtex bibliography
