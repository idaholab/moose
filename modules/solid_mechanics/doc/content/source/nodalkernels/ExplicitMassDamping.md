# Explicit Mass Damping

!syntax description /NodalKernels/ExplicitMassDamping

# Description

The purpose of this [NodalKernel](NodalKernel.md) is to introduce Rayleigh mass damping into an [ExplicitMixedOrder](ExplicitMixedOrder.md) solid-mechanics model.  Rayleigh mass damping is used to damp low frequency, long wavelength vibratory modes of the model.  This is particularly useful in models where quasi-static solutions are of interest, as the low frequency modes can persist for many time steps if not damped.  Related aside: high frequency, short wavelength vibratory models (eg, on the scale of a single element) are damped using `stiffness_damping_coefficient` in [DynamicSolidMechanicsPhysics](DynamicSolidMechanicsPhysics.md).

`ExplicitMassDamping` computes the residual contribution $\eta M v_{\mathrm{old}}$, where $\eta$ is a parameter chosen by the user, $M$ is the nodal mass, and $v_{\mathrm{old}}$ is the old value of the velocity.  The nodal mass, $M$, must have been calculated by a [MassMatrix](MassMatrix.md) Kernel, and its value depends on the `density` supplied to that Kernel.  Usually, an `ExplicitMassDamping` Kernel should be attached to each of the displacement variables in the system (eg, `disp_x`, `disp_y`, `disp_z`).

Choosing an appropriate value of `eta` sometimes requires experimentation.  A method can be found [here](solid_mechanics/Dynamics.md).  Alternately, here are some rough guidelines, noting that `ExplicitMassDamping` will damp low-frequency oscillations of your model.

Firstly, calculate the frequency, $\omega$ (rad/s), of the oscillations you wish to damp most effectively (`ExplicitMassDamping` will also damp other frequencies, but less efficiently).  For instance:

- If longitudinal waves (sometimes called "compressional waves") of half-wavelength $L$ are annoying, then $\omega = \pi L^{-1} \sqrt(E / \rho)$.  Often $L$ is similar to the size of your model.
- If your model is a bar of length $L$ that is clamped at one end, and you wish to damp the fundamental frequency oscillations, then these oscillations have $\omega \approx L^{-2}\sqrt{E I / (\rho A)}.

In these, $E$ is the Young's modulus, $\rho$ the density, $I$ the bar's cross-sectional moment of inertia, and $A$ the bar's cross-sectional area.

Secondly, calculate the frequency, $\omega_{\mathrm{high}}$ of the high-frequency oscillations that you wish to damp.  Usually, these are $\omega_{\mathrm{high}} \sim \mathrm{Max}_{\mathrm{element}\pi L_{\mathrm{element}}^{-1} \sqrt{E_{\mathrm{element}} / \rho_{\mathrm{element}})$, where the maximum is over all the elements in the model, and $L_{\mathrm{element}}$ is the element side length.

Thirdly, choose the damping ratio, $d$, which is a measure of how quickly oscillations decay relative to critical damping.  Usually $d = 0.1$ is a reasonable choice.

Fourthly, set $\eta = 2d \omega \omega_{\mathrm{high}} / (\omega + \omega_{\mathrm{high}})$.  Usually $\omega_{\mathrm{high}} \gg \omega$, so this reduces to $\eta \approx 2d\omega$ (with units 1/s).  In addition, set the `stiffness_damping_coefficient` to $2d/(\omega + \omega_{\mathrm{high}}) \approx 2d / \omega_{\mathrm{high}}$ (with units s).

### !listing modules/solid_mechanics/test/tests/beam/dynamic/nodal_mass.csv

!syntax parameters /NodalKernels/ExplicitMassDamping

!syntax inputs /NodalKernels/ExplicitMassDamping

!syntax children /NodalKernels/ExplicitMassDamping
