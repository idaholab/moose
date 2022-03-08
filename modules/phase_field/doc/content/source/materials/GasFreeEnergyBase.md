# GasFreeEnergyBase

$n_Q$ is the _quantum concentration_ defined by

\begin{equation}
n_Q = \left(\frac{2\pi m k_B T}{h^2}\right)^\frac32.
\end{equation}

Here $m$ (`m`) is the gas atom mass, which by default us specified in atomic mass
units $u$ (this can be customized using the `mass_unit_conversion`). $T$ (`T`) is
the coupled temperature, $n$ is defined as

\begin{equation}
n= \frac NV = \frac c\Omega,
\end{equation}

where $c$ (`c`) is the coupled concentration and $\Omega$ (`omega`) is the lattice
site volume.
