# FVReconstructedPressureGradient

## Description

`FVReconstructedPressureGradient` implements the Aguerre face-flux reconstruction
([!cite](aguerre2018oscillation)) used by the linear finite-volume segregated solver. It owns the
lagged velocity-gradient snapshot, reconstructed pressure-gradient candidate, and relaxed coupling
pressure gradient used by the momentum predictor.
The snapshot is copied from each velocity variable's ordinary configured gradient field after the
previous pressure corrector, so this object does not implement a separate velocity-gradient
algorithm.

After each pressure corrector, [RhieChowMassFlux.md] supplies the corrected conservative face flux,
$\mathbf{H}/\mathbf{A}$, $\mathbf{A}^{-1}$, and momentum/pressure system metadata. The method removes
the lagged Taylor contribution from the face flux and solves a local face-to-cell projection for a
compatible cell velocity. It then recovers the pressure gradient satisfying

\begin{equation}
\mathbf{u}_P = -\left(\frac{\mathbf{H}}{\mathbf{A}}\right)_P
               -\mathbf{A}^{-1}_P\left(\nabla p\right)_P.
\end{equation}

The candidate is formed before pressure relaxation from the unrelaxed pressure gradient and its
conservative corrected face flux. It is used directly for the immediate cell-velocity correction,
so that correction remains compatible with the face flux. After the relaxed pressure solution is
installed, the reconstructed candidate is blended into the published coupling field using
[!param](/FVGradientMethods/FVReconstructedPressureGradient/gradient_relaxation). Before the first
candidate of the simulation is available, the method publishes
[!param](/FVGradientMethods/FVReconstructedPressureGradient/base_gradient_method), such as
[FVGreenGaussGradient.md]. Because the method retains flow-system state, each instance can be used
by only one `RhieChowMassFlux`, pressure system, pressure variable, and set of momentum systems. The
coupling pressure gradient is retained between accepted time steps so a steady solution does not
acquire an artificial momentum residual from switching back to the base gradient. The accepted
field is stored as the old linear FV gradient state, which restores it when a time step is retried
and includes it in restart data. Candidate fields and generation counters are reset once per
attempt, while allocated vector storage is reused when its layout remains valid.

This method is intended specifically for momentum-pressure coupling. Diffusion corrections,
diagnostics, and unrelated equations should continue to use an ordinary gradient method.

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
