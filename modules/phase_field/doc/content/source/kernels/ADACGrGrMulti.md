# ADACGrGrMulti

!syntax description /Kernels/ADACGrGrMulti

This kernel is the AD version of [ACGrGrMulti](\ACGrGrMulti). It  calculates the residual for grain growth for a multi-phase,
poly-crystal system
\begin{equation}
\left(L\mu(\eta_i^3-\eta_i+2\eta_i\sum_{j\neq i} \gamma_{ij}\eta_j^2),\psi\right),
\end{equation}
where $L$ is the mobility, $\eta_i$ the kernel variable, and $\eta_j$ are the
other order parameters. $\mu$ and $\gamma$ are model parameters contributing to
the grain boundary energy.
A list of material properties needs to be supplied for the $\gamma_{ij}$
(prefactors of the cross-terms between order parameters) term.

!syntax parameters /Kernels/ADACGrGrMulti

!syntax inputs /Kernels/ADACGrGrMulti

!syntax children /Kernels/ADACGrGrMulti

!bibtex bibliography
