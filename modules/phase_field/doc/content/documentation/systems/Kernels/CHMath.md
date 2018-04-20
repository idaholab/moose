# CHMath

!syntax description /Kernels/CHMath

Cahn-Hilliard bulk kernel with a hardcoded free energy density $f$
\begin{equation}
f=\frac14c^4-\frac12c^2,
\end{equation}
where $c$ is the conserved non-linear order parameter variable the kernel is acting
on. The free energy density is minimized at $c=\pm1$.

!syntax parameters /Kernels/CHMath

!syntax inputs /Kernels/CHMath

!syntax children /Kernels/CHMath
