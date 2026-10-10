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

To compute the parameters, a reference interface is assigned
$\gamma_{\alpha i \beta j}=1.5$. By default this is the interface with the
largest $\sigma_{\alpha i \beta j}$ (`reference_sigma = max`), following the
parameterization procedure of [!cite](Moelans2022). Alternatively the interface
with the median $\sigma_{\alpha i \beta j}$ (`reference_sigma = median`), or the
$\sigma_{\alpha i \beta j}$ entry with the index `sigma_index` is chosen
(overriding `reference_sigma`). For this gamma value a set of analytical
expressions holds

\begin{equation}
\begin{aligned}
\kappa &= \frac 34 \sigma_{\alpha i \beta j} l_{\alpha i \beta j}\\
\mu &= \frac{6 \sigma_{\alpha i \beta j}} {l_{\alpha i \beta j}}.
\end{aligned}
\end{equation}

!alert note title=Interface widths
Note that the interface width $l$ (`width`) is only guaranteed for the reference
interface. All other interface widths are a function of their respective
interfacial free energies, and interfaces with a lower interfacial free energy
are wider. With the default `reference_sigma = max` the reference interface is
therefore the narrowest one.

With $\kappa$ and $\mu$ determined the remaining $\gamma_{\alpha i \beta j}$ can
be computed using the fitted relation [!cite](Moelans2022)

\begin{equation}
\begin{aligned}
g_{\alpha i \beta j} &= \frac{\sigma_{\alpha i \beta j}}{\sqrt{\mu\kappa}}  \\
\gamma_{\alpha i \beta j} &= \left( -3.0944 g_{\alpha i \beta j}^8 -1.8169 g_{\alpha i \beta j}^6 + 10.323 g_{\alpha i \beta j}^4 -8.1819 g_{\alpha i \beta j}^2 + 2.0033 \right)^{-1}.
\end{aligned}
\end{equation}

This fit is valid for $0.53 \le \gamma_{\alpha i \beta j} \le 40$, i.e.
$0.098 \le g_{\alpha i \beta j} \le 0.766$, and an error is reported for
interfacial free energies outside this range. The earlier fit
[!cite](MoelansWeb)

\begin{equation}
\gamma_{\alpha i \beta j} = \left( -5.288 g_{\alpha i \beta j}^8 -0.09364 g_{\alpha i \beta j}^6 + 9.965 g_{\alpha i \beta j}^4 -8.183 g_{\alpha i \beta j}^2 + 2.007 \right)^{-1}
\end{equation}

can be selected with `interface_fit = moelans2009` to reproduce earlier results.
It is accurate for $\gamma_{\alpha i \beta j} \lesssim 5$, loses accuracy above
that, and diverges near $\gamma_{\alpha i \beta j} \approx 34$.

The material propertied provided by this class are directly used by the
[`ACGrGrMulti`](/ACGrGrMulti.md) and [`ACInterface`](/ACInterface.md) objects
and indirectly used by the
[GrandPotentialKernelAction](/actions/GrandPotentialKernelAction.md).

!syntax parameters /Materials/GrandPotentialInterface

!syntax inputs /Materials/GrandPotentialInterface

!syntax children /Materials/GrandPotentialInterface

!bibtex bibliography
