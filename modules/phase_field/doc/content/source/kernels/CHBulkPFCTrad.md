# CHBulkPFCTrad

!syntax description /Kernels/CHBulkPFCTrad

Cahn-Hilliard kernel implementing the free energy density
\begin{equation}
f = \frac12c^2\cdot(1-C_0)-\frac16ac^3+\frac1{12}bc^4,
\end{equation}
where $c$ is a PFC density and the coefficients $C_0,a,b$ are provided by
the [`PFCTradMaterial`](PFCTradMaterial.md).

!syntax parameters /Kernels/CHBulkPFCTrad

!syntax inputs /Kernels/CHBulkPFCTrad

!syntax children /Kernels/CHBulkPFCTrad
