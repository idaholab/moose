# ADACBarrierFunction

!syntax description /Kernels/ADACBarrierFunction

This kernel is the AD version of [ACBarrierFunction](/ACBarrierFunction). It implements the following term

\begin{equation}
\left(L\frac{\partial \mu}{\partial \eta_i}(\eta_i^4-\eta_i^2+2\gamma\sum_i\sum_{j\neq i}\eta_i^2\eta_j^2),\psi\right),
\end{equation}

where $L$ is the mobility, $\eta_i$ the kernel variable, and $\eta_j$ are the
other order parameters. $\mu$ and $\gamma$ are model parameters contributing to
the grain boundary energy. This is applicable only when $\mu$ is a function of the order parameters. It currently only takes a single value for gamma.

!syntax parameters /Kernels/ADACBarrierFunction

!syntax inputs /Kernels/ADACBarrierFunction

!syntax children /Kernels/ADACBarrierFunction

!bibtex bibliography
