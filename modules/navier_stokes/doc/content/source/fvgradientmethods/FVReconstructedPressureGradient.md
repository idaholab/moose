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

After each pressure corrector, [RhieChowMassFlux.md] provides the corrected conservative
volumetric face flux together with the diagonal momentum-balance coefficients $\mathbf{H}/\mathbf{A}$
and $\mathbf{A}^{-1}$ used by its Rhie-Chow interpolation. For a cell $P$ with faces $f$, let
$\mathbf{S}_f = |\mathbf{S}_f|\,\mathbf{n}_f$ be the face area vector outward from $P$,
$\mathbf{d}_{Pf}$ the vector from the cell centroid to the face centroid, and $q_f$ the corrected
volumetric flux through $f$, signed positive out of $P$. The reconstruction proceeds as follows:

1. Freeze the velocity gradient. Before the pressure corrector changes the face flux, the current
   cell-centered velocity gradient $(\nabla\mathbf{u})_P$ is saved as a lagged reference,
   $(\nabla\mathbf{u})^{\text{lag}}_P$. This keeps the reconstruction from depending on the same
   velocity field it is trying to update. At an internal face, the lagged gradients of the two
   neighboring cells are averaged to a face value $(\nabla\mathbf{u})^{\text{lag}}_f$; at a domain
   boundary, or at the edge of this method's block restriction, the owning cell's lagged gradient
   is used directly.

2. Turn each corrected face flux into a face equation for the cell velocity. A first-order Taylor
   expansion of the velocity from the cell centroid to the face centroid gives

   \begin{equation}
   \mathbf{u}_f\cdot\mathbf{n}_f = \mathbf{u}_P\cdot\mathbf{n}_f
     + \left[(\nabla\mathbf{u})^{\text{lag}}_f\,\mathbf{d}_{Pf}\right]\cdot\mathbf{n}_f .
   \end{equation}

   Replacing $\mathbf{u}_f\cdot\mathbf{n}_f$ with the corrected face-normal velocity
   $q_f/|\mathbf{S}_f|$ and moving the already-known Taylor term to the right-hand side gives, for
   every face of $P$, one linear equation for the unknown cell velocity,

   \begin{equation}
   \mathbf{u}_P\cdot\mathbf{n}_f \approx \hat{q}_f \equiv \frac{q_f}{|\mathbf{S}_f|}
     - \left[(\nabla\mathbf{u})^{\text{lag}}_f\,\mathbf{d}_{Pf}\right]\cdot\mathbf{n}_f .
   \end{equation}

3. Project the face equations back to a cell velocity. A cell generally has more faces than
   spatial dimensions, so its face equations are combined in an area-weighted least-squares sense:

   \begin{equation}
   \left[\sum_{f} |\mathbf{S}_f|\,\mathbf{n}_f\mathbf{n}_f^T\right]\mathbf{u}_P
     = \sum_{f} |\mathbf{S}_f|\,\hat{q}_f\,\mathbf{n}_f .
   \end{equation}

   This normal-equation system is symmetric positive definite whenever the face normals of $P$
   span the spatial dimension. Its solution $\mathbf{u}_P$ is the cell velocity most consistent,
   in this least-squares sense, with every corrected face flux around the cell.

4. Invert the momentum balance for the pressure gradient. The discrete momentum balance used by the
   momentum predictor relates the cell velocity to the pressure gradient through the coefficients
   supplied by [RhieChowMassFlux.md],

   \begin{equation}
   \mathbf{u}_P = -\left(\frac{\mathbf{H}}{\mathbf{A}}\right)_P
                  -\mathbf{A}^{-1}_P\left(\nabla p\right)_P.
   \end{equation}

   Substituting, for each spatial direction $i$, the reconstructed velocity component $u_{P,i}$
   found in step 3 and solving for the unknown gradient component gives

   \begin{equation}
   \left(\nabla p\right)_{P,i} = -\frac{u_{P,i} + (H/A)_{P,i}}{A^{-1}_{P,i}} .
   \end{equation}

The pressure gradient found in step 4 is used immediately to update the cell velocity, so the
velocity reported at cell centers stays consistent with the flux that satisfies continuity at
faces. A relaxed blend of this reconstructed gradient and the previous coupling gradient is then
carried forward to the next momentum predictor,

\begin{equation}
\mathbf{g}^{k+1}_{\text{coupling}} = (1-\alpha)\,\mathbf{g}^{k}_{\text{coupling}}
  + \alpha\,\mathbf{g}^{k}_{\text{reconstructed}},
\end{equation}

where $\alpha$ is [!param](/FVGradientMethods/FVReconstructedPressureGradient/gradient_relaxation).
This relaxation controls the strength of the pressure-velocity feedback without changing the
conservative face flux produced by the current pressure correction.

## Behavior During PISO Correctors

[PIMPLE.md] can run several pressure correctors after one momentum predictor, without
reassembling the momentum system between them, using the number of PISO iterations set by
[!param](/Executioner/PIMPLE/num_piso_iterations). The momentum matrix and its $\mathbf H/\mathbf A$
and $\mathbf A^{-1}$ coefficients stay fixed for that entire PISO sequence; see [PIMPLE.md] for that
part of the algorithm. The reconstruction, however, repeats steps 1-4 above once for every
corrector, not once for the whole sequence:

- Every corrector, including intermediate ones, recomputes the conservative face flux. An ordinary
  gradient method only needs the flux from the final corrector to advance the advection terms, but
  the reconstruction always needs the flux produced by its own pressure solve to build a new
  candidate, so this cost is paid at every corrector.
- Step 1 (freezing $(\nabla\mathbf u)^{\text{lag}}_P$) is repeated before every corrector, using the
  cell velocity produced by the previous corrector, or, for the first corrector, by the previous
  momentum predictor. Freezing the gradient once per corrector, rather than once per momentum
  predictor, keeps each corrector's reconstruction from depending on a velocity field that it is
  about to correct again.
- Steps 2 through 4 then run against that corrector's own corrected flux and lagged gradient. The
  resulting pressure-gradient candidate is used immediately, unrelaxed, to update the cell velocity
  for that corrector, exactly as described above.
- The coupling gradient is also relaxed and updated after every corrector, blending in that
  corrector's own candidate. With $C$ correctors producing candidates
  $\mathbf g^{(0)}_{\text{reconstructed}},\dots,\mathbf g^{(C-1)}_{\text{reconstructed}}$ in order,
  and $\mathbf g_{\text{coupling}}$ the coupling gradient carried in from the previous momentum
  predictor, the value carried forward to the next momentum predictor is the sequential blend

  \begin{equation}
  \mathbf g_{\text{coupling}}^{\text{next}} = (1-\alpha)^{C}\,\mathbf g_{\text{coupling}}
    + \alpha\sum_{k=0}^{C-1}(1-\alpha)^{C-1-k}\,\mathbf g^{(k)}_{\text{reconstructed}} ,
  \end{equation}

  so the candidate from the last corrector carries the most weight, but earlier correctors' candidates
  are not discarded.

Because the flux recomputation and the per-cell least-squares solve repeat at every corrector, using
this gradient method with a nonzero [!param](/Executioner/PIMPLE/num_piso_iterations) costs more per
momentum predictor than an ordinary gradient method does. In exchange, the cell velocity and pressure
gradient stay mutually consistent through every corrector, rather than only at the end of the PISO
sequence.

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

The pressure and velocity variables and their `RhieChowMassFlux` object must have identical block
restrictions. Fields on independent flow regions should use separate variables and Rhie-Chow
objects, or one Rhie-Chow object must span the complete flow domain.

Mesh quality is outside this gradient method's responsibility. When diagnosing reconstruction on a
new mesh, use [MeshDiagnosticsGenerator.md] with `check_local_jacobian = ERROR`,
`examine_element_volumes = ERROR` and `examine_non_conformality = ERROR` to detect degenerate element and side geometry.

## Example Input File Syntax

The following block defines a `FVReconstructedPressureGradient` for the pressure variable in a
linear finite-volume segregated solve:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/reconstructed-force-channel.i block=FVGradientMethods/reconstructed

The pressure variable then selects this gradient method by name:

!listing modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d/reconstructed-force-channel.i block=Variables/pressure

!syntax parameters /FVGradientMethods/FVReconstructedPressureGradient

!syntax inputs /FVGradientMethods/FVReconstructedPressureGradient

!syntax children /FVGradientMethods/FVReconstructedPressureGradient
