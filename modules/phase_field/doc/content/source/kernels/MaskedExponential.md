# MaskedExponential

!syntax description /Kernels/MaskedExponential

This kernel implements a term in the variationally-derived equivalent form of Poisson's equation
for the electrochemical grand potential sintering model with dilute solution energetics.
It represents the contribution from vacancy species $i$ with state of charge $Z$.
The weak form of this term is
\begin{equation}
h_p Z e n_{v,i} ^{eq} \exp{ \left( \frac{\mu_i - ZeV}{kT} \right) }
\end{equation}
where $h_p$ is the switching function for phase $p$ (which has a value of 1 in phase $p$ and 0 outside that phase),
$Z$ is the state of charge of the vacancy, $e$ is the fundamental electron charge, $n_{v,i} ^{eq}$ is the equilibrium vacancy concentration of species $i$, $\mu_i$ is the chemical potential of species $i$, $V$ is the electric
potential, $k$ is Boltzmann's constant, and $T$ is the temperature in K.

!syntax parameters /Kernels/MaskedExponential

!syntax inputs /Kernels/MaskedExponential

!syntax children /Kernels/MaskedExponential

!bibtex bibliography
