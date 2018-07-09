# SimpleSplitCHWRes

!syntax description /Kernels/SimpleSplitCHWRes

!include simple_kernels_include.md

This kernel implements the weak form
\begin{equation}
(M \nabla u, \nabla\psi)
\end{equation}
for the a reverse split Cahn-Hilliard equation. For a more feature complete
version see the [`SplitCHWRes`](/SplitCHWRes.md) kernel, which computes a full Jacobian
even for variable dependent mobilities $M$.

!syntax parameters /Kernels/SimpleSplitCHWRes

!syntax inputs /Kernels/SimpleSplitCHWRes

!syntax children /Kernels/SimpleSplitCHWRes

!bibtex bibliography
