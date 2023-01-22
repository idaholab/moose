# NumericalFlux3EqnDGKernel

!syntax description /DGKernels/NumericalFlux3EqnDGKernel

This DG kernel implements the side flux contributions for the 1-D, 1-phase, variable-area
Euler equations. It uses a provided numerical flux user object, which computes the
side flux given the left and right states:
\begin{equation}
  \mathbf{F}_{i+1/2} = \mathcal{F}(\mathbf{U}_i, \mathbf{U}_{i+1}, \mathbf{n}_{i+1/2}) \,,
\end{equation}
where the solution vectors here are the reconstructed solutions from the
adjacent cells.

!syntax parameters /DGKernels/NumericalFlux3EqnDGKernel

!syntax inputs /DGKernels/NumericalFlux3EqnDGKernel

!syntax children /DGKernels/NumericalFlux3EqnDGKernel
