# FVReconstructedPressureGradient

## Description

`FVReconstructedPressureGradient` implements the Aguerre face-flux reconstruction
([!cite](aguerre2018oscillation)) used by the linear finite-volume segregated solver. It owns the
lagged velocity-gradient snapshot, reconstructed pressure-gradient candidate, and relaxed feedback
field used by the momentum predictor.

After each pressure corrector, [RhieChowMassFlux.md] supplies the corrected conservative face flux,
$\mathbf{H}/\mathbf{A}$, $\mathbf{A}^{-1}$, and momentum/pressure system metadata. The method removes
the lagged Taylor contribution from the face flux and solves a local face-to-cell projection for a
compatible cell velocity. It then recovers the pressure gradient satisfying

\begin{equation}
\mathbf{u}_P = -\left(\frac{\mathbf{H}}{\mathbf{A}}\right)_P
               -\mathbf{A}^{-1}_P\left(\nabla p\right)_P.
\end{equation}

The reconstructed candidate is blended into the published coupling field using
[!param](/FVGradientMethods/FVReconstructedPressureGradient/gradient_relaxation). Before the first
candidate of a time step is available, the method publishes
[!param](/FVGradientMethods/FVReconstructedPressureGradient/base_gradient_method), such as
[FVGreenGaussGradient.md]. The feedback and generation counters are reset once per attempted time
step, while allocated vector storage is reused when its layout remains valid.

This method is intended specifically for momentum-pressure coupling. Diffusion corrections,
diagnostics, and unrelated equations should continue to use an ordinary gradient method.

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
