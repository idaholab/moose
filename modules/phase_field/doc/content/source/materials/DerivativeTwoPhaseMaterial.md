# DerivativeTwoPhaseMaterial

!syntax description /Materials/DerivativeTwoPhaseMaterial

The simplified **two-phase model** uses a single order parameter to switch between the two phases. A global free energy is constructed using a meta material class that combines the phase free energies.

For two phase models the `DerivativeTwoPhaseMaterial` can be used to combine two phase
free energies into a global free energy (which the [`AllenCahn`](/AllenCahn.md)
and [`Cahn-Hilliard`](/CahnHilliard.md) kernels use to evolve the system) as

\begin{equation}
F = \left(1-h(\eta)\right) F_a + h(\eta)F_b + Wg(\eta)
\end{equation}

!syntax parameters /Materials/DerivativeTwoPhaseMaterial

!syntax inputs /Materials/DerivativeTwoPhaseMaterial

Note that the phase free energies usually have single well character. The global free energy
landscape will however have a double well character in the examples above.

!syntax children /Materials/DerivativeTwoPhaseMaterial
