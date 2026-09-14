# FVReconstructedPressureGradient

## Description

`FVReconstructedPressureGradient` implements the Aguerre face-flux reconstruction
([!cite](aguerre2018oscillation)) for the linear finite-volume segregated solver. On a collocated
mesh, the pressure correction enforces continuity through conservative face fluxes, while the
momentum equations are solved for cell-centered velocities. If the cell pressure gradient is not
consistent with the corrected face flux, the face flux can be smooth and conservative while the
cell velocity remains oscillatory. The reconstruction reduces this mismatch by finding the cell
pressure gradient implied by the corrected face flux and the discrete momentum balance.

## Reconstruction Algorithm

After each pressure corrector, [RhieChowMassFlux.md] provides the corrected conservative face flux,
$\mathbf{H}/\mathbf{A}$, and $\mathbf{A}^{-1}$. The reconstruction proceeds as follows:

1. The velocity gradient from the previous correction estimates how velocity varies from each cell
   center to its faces.
2. That variation is removed from the corrected face-normal volumetric fluxes, and the remaining
   face information is projected back to a cell-centered velocity.
3. The pressure gradient is calculated so that the reconstructed cell velocity satisfies the same
   discrete momentum balance used by the momentum predictor,

\begin{equation}
\mathbf{u}_P = -\left(\frac{\mathbf{H}}{\mathbf{A}}\right)_P
               -\mathbf{A}^{-1}_P\left(\nabla p\right)_P.
\end{equation}

The pressure gradient associated with the current corrected face flux is used immediately to update
the cell velocity. This preserves consistency between the velocity reported at cell centers and the
flux that satisfies continuity at faces. A relaxed blend of the reconstructed gradient and the
previous coupling gradient is then used in the next momentum predictor. The relaxation controls the
strength of this pressure-velocity feedback without changing the conservative face flux from the
current pressure correction.

## Initial and Transient Behavior

Before the first pressure correction, no reconstructed gradient exists, so the momentum predictor
uses [!param](/FVGradientMethods/FVReconstructedPressureGradient/base_gradient_method), which
defaults to [FVGreenGaussGradient.md]. After a time step is accepted, its relaxed reconstructed
gradient becomes the starting point for the next time step. A rejected time-step attempt restores
the last accepted gradient, and restart data preserves the same accepted state. This avoids creating
an artificial momentum imbalance merely by advancing, retrying, or restarting a converged solution.

The relaxation and initialization choices are described in
[RhieChowMassFlux.md#reconstructed-pressure-gradient].

## Intended Use

This method is intended specifically for momentum-pressure coupling. Diffusion corrections,
diagnostics, and unrelated equations should continue to use an ordinary gradient method. Use the
same reconstructed pressure-gradient definition for every momentum component coupled to one
pressure equation; independent flow systems should use separate definitions.

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
