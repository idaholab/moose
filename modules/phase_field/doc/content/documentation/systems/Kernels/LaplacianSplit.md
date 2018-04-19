# LaplacianSplit

!syntax description /Kernels/LaplacianSplit

Implements the weak form residual
\begin{equation}
\left(\nabla c,\nabla\psi\right)
\end{equation}
for the strong form
\begin{equation}
-\nabla^2c.
\end{equation}

Used together with a [`Reaction`](/Reaction.md) kernel $u$ this allows the construction
of split formulations,
\begin{equation}
u=\nabla^2c,
\end{equation}
where $u$ can be substituted to reduce the order of a PDE.

!syntax parameters /Kernels/LaplacianSplit

!syntax inputs /Kernels/LaplacianSplit

!syntax children /Kernels/LaplacianSplit
