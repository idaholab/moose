# Porous SIMPLE Formulation

`PorousRhieChowMassFlux` and `LinearPNSFVMomentumPressure` extend the
segregated linear finite-volume solver to the porous media equations

$$
\nabla \cdot (\epsilon \rho \mathbf{u} \mathbf{u})
 - \nabla \cdot (\epsilon \mu \nabla \mathbf{u})
 = - \epsilon \nabla p + \mathbf{f},
\qquad
\nabla \cdot (\epsilon \rho \mathbf{u}) = 0,
$$

where $\mathbf{u}$ is the interstitial velocity and $\epsilon$ is the porosity.
The modifications relative to the non-porous SIMPLE implementation are:

1.  The Rhie–Chow interpolation stores $\epsilon \rho$ in both the $H/A$ and
    $A^{-1}$ functors, which makes the advective fluxes and the pressure
    correction consistent with $\nabla \cdot (\epsilon \rho \mathbf{u}) = 0$.
2.  The pressure gradient contribution in the momentum predictor is multiplied
    by the local porosity.
3.  Optional Bernoulli jumps and user-defined losses can be injected at block
    interfaces to represent empirical head changes.

Users should provide a viscosity functor that already contains the porosity
factor (e.g. through an `ADParsedFunctorMaterial` or a parsed function)
whenever the diffusion term is required.

## Porosity Interpolation Strategy

Face porosities are evaluated with harmonic weighting by default to maintain
continuity of the superficial flux when the interstitial velocity is allowed
to jump between blocks.  Arithmetic and geometric options are available
through the `porosity_face_interpolation` parameter for cases where a different
scheme is preferable.

## Interface Pressure Jumps

When a face is tagged (either automatically by detecting $\epsilon$ jumps
between neighboring blocks or manually through `porosity_jump_sidesets`) the
user object computes a Bernoulli correction

$$ \Delta p = \frac{1}{2}\rho_d v_d^2 - \frac{1}{2}\rho_u v_u^2, $$

where the subscripts denote upwind and downwind states along the face normal.
Form factors supplied in `interface_form_factors` add irreversible losses based
on the downwind superficial velocity for contractions and the upwind velocity
for expansions, while `interface_pressure_jumps` allows the user to prescribe
additional constant drops.  The resulting pressure jump is subtracted from the
downwind pressure unknown before the face gradient is evaluated, which embeds
the loss in both the predictor and corrector steps without special kernels.
