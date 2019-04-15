# ADSplitCHParsed

!syntax description /Kernels/ADSplitCHParsed

Implements the weak form

\begin{equation}
(\frac{\partial F}{\partial c}, \psi) - (\mu,\psi) + (\kappa\nabla c,\nabla\psi)
\end{equation}

where $c$ is the concentration variable the kernel is operating on, $F$ is a
free energy material property (with accompanying derivative properties), $\mu$
is the chemical potential variable, and $\kappa$ is the gradient energy
coefficient.

It is used together with [`ADSplitCHWRes`](/ADSplitCHWRes.md) and
[`ADCoupledTimeDerivative`](/ADCoupledTimeDerivative.md) to set up a system of
two first order PDEs using a concentration order parameter and a chemical
potential variable.

!syntax parameters /Kernels/ADSplitCHParsed

!syntax inputs /Kernels/ADSplitCHParsed

!syntax children /Kernels/ADSplitCHParsed

!bibtex bibliography
