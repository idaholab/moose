# NodalVoidVolume

This UserObject computes the void volume associated with each node $i$, which is
\begin{equation}
V_{i} = \int_{V}\phi \ ,
\end{equation}
where:

- $\phi$ is the porosity
- $V$ is the volume surrounding a node

In finite-element language, this is
\begin{equation}
V_{i} = \int_{\mathrm{elements}} \psi_{i}\phi \ ,
\end{equation}
where $\psi_{i}$ is the shape function for node $i$, and the integral is over all elements attached to the node.

$V_{i}$ is allows translation between moles of reactant at a node, $M_{i}$, and concentration (mol/volume-of-solute) at the node:
\begin{equation}
\mathrm{concentration}_{i} = \frac{M_{i}}{V_{i}} \ .
\end{equation}

$V_{i}$ may be recorded into an AuxVariable using [NodalVoidVolumeAux](NodalVoidVolumeAux.md).


!syntax parameters /UserObjects/NodalVoidVolume

!syntax inputs /UserObjects/NodalVoidVolume

!syntax children /UserObjects/NodalVoidVolume
