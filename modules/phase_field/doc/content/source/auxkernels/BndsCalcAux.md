# BndsCalcAux

!syntax description /AuxKernels/BndsCalcAux

This AuxKernel computes the value $v$ as

\begin{equation}
b = \sum_i v_i^2
\end{equation}

where $b$ is the variable the AuxKernel is acting on and $v_i$ (`v`) are a set
of couple order parameters. When used with order parameters of the
polycrystalline grain growth model, the resulting field for $b$ is $1$ in grain
interiors and $<1$ in grain boundaries ($\frac14$ along the mid plan of the
grain boundaries).

!syntax parameters /AuxKernels/BndsCalcAux

!syntax inputs /AuxKernels/BndsCalcAux

!syntax children /AuxKernels/BndsCalcAux

!bibtex bibliography
