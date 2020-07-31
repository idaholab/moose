# NodalVoidVolumeAux

This AuxKernel extracts the void volume, $V_{i}$, associated with each node $i$ from a [NodalVoidVolume](NodalVoidVolume.md) UserObject.  Its `variable` must therefore be a nodal variable (not a monomial, for instance).

$V_{i}$ is allows translation between moles of reactant at a node, $M_{i}$, and concentration (mol/volume-of-solute) at the node:
\begin{equation}
\mathrm{concentration}_{i} = \frac{M_{i}}{V_{i}} \ .
\end{equation}


!syntax parameters /AuxKernels/NodalVoidVolumeAux

!syntax inputs /AuxKernels/NodalVoidVolumeAux

!syntax children /AuxKernels/NodalVoidVolumeAux
