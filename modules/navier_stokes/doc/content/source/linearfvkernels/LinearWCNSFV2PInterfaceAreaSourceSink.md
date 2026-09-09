# LinearWCNSFV2PInterfaceAreaSourceSink

Sources and sinks of the interfacial area concentration transport equation, for the linear finite
volume discretization. This is the one-group transport equation, assembled in conservative form.
Both closure sets are implemented and are selected with the
[!param](/LinearFVKernels/LinearWCNSFV2PInterfaceAreaSourceSink/model) parameter.

## The transport equation

With $\rho_g$, $\alpha_g$ and $\bm{u}_g$ the density, volume fraction
and velocity of the dispersed phase and $\dot m_g$ the mass transfer rate into it per unit mixture
volume:

\begin{equation}
\frac{\partial (\rho_g \xi)}{\partial t}
  + \nabla \cdot \left(\rho_g \bm{u}_g \xi\right)
  = \frac{1}{3}\frac{D\rho_g}{Dt}\xi
  + \frac{2}{3}\frac{\dot m_g}{\alpha_g}\xi
  + \rho_g\left(S_{RC} + S_{WE} + S_{TI}\right)
\end{equation}

This object assembles the right hand side only. The two terms on the left are the business of
[LinearFVTimeDerivative.md], given a `factor` of $\rho_g$, and of [LinearFVScalarAdvection.md],
given a `density` of $\rho_g$ and the dispersed phase velocity
$\bm{u}_g = \bm{u}_m + \bm{u}_{Md}$. Assembling the equation in any other form, in particular
without the density on both, is inconsistent with the sources computed here.

The averaged particle size common to both closure sets is
$d_b = \psi \alpha_g / \xi$, with $\psi = 6$ for spherical particles.

## Hibiki and Ishii

From [!cite](hibiki2000interface). There is no wake entrainment model in this formulation, so
$S_{WE} = 0$. With $\Pi$ the prefactor the two terms share,

\begin{equation}
\Pi = \left(\frac{\alpha_g}{\xi}\right)^{2}
      \frac{\epsilon^{1/3}}{d_b^{11/3}\left(\alpha_{g,max}-\alpha_g\right)}
\end{equation}

\begin{equation}
S_{RC} = -\Pi \Gamma_C \alpha_g^2
  \exp\left(-K_C \frac{d_b^{5/6}\rho_f^{1/2}\epsilon^{1/3}}{\sigma^{1/2}}\right)
\end{equation}

\begin{equation}
S_{TI} = \Pi \Gamma_B \alpha_g \left(1-\alpha_g\right)
  \exp\left(-K_B \frac{\sigma}{\rho_f d_b^{5/3}\epsilon^{2/3}}\right)
\end{equation}

with $\Gamma_C = 0.188$, $K_C = 0.129$, $\Gamma_B = 0.264$ and $K_B = 1.37$, the values given by the
reference and the defaults of this object. The packing factor divides, so both terms grow without
bound as the dispersed phase approaches its maximum.

## Ishii and Kim

With the mean bubble fluctuating velocity
$u_t = \epsilon^{1/3} d_b^{1/3}$ and the Weber number $We = \rho_f u_t^2 d_b / \sigma$:

\begin{equation}
S_{RC} = -\frac{1}{3\pi} C_{RC} u_t \xi^2
  \frac{1}{\alpha_{g,max}^{1/3}\left(\alpha_{g,max}^{1/3}-\alpha_g^{1/3}\right)}
  \left[1 - \exp\left(-C \frac{\alpha_{g,max}^{1/3}\alpha_g^{1/3}}
                              {\alpha_{g,max}^{1/3}-\alpha_g^{1/3}}\right)\right]
\end{equation}

\begin{equation}
S_{WE} = -\frac{1}{3\pi} C_{WE} u_r \xi^2 C_D^{1/3}
\qquad
S_{TI} = \frac{1}{18} C_{TI} u_t \frac{\xi^2}{\alpha_g}
  \left(1-\frac{We_{cr}}{We}\right)^{1/2}\exp\left(-\frac{We_{cr}}{We}\right)
\end{equation}

with $C_{RC} = 0.004$, $C_{WE} = 0.002$, $C_{TI} = 0.085$, $C = 3.0$, $We_{cr} = 6.0$ and
$\alpha_{g,max} = 0.75$. As the reference specifies, the breakage rate is zero below the critical
Weber number.

The terminal velocity and the drag coefficient are mutually
implicit: $u_r$ depends on $C_D$, and $C_D$ on $Re_D = \rho_f u_r d_b (1-\alpha_g)/\mu_f$.
Eliminating $C_D$ leaves the scalar equation

!equation
u_r \left(1 + \frac{1}{10}\left(B u_r\right)^{3/4}\right) = T,
\qquad B = \frac{\rho_f d_b (1-\alpha_g)}{\mu_f},
\qquad T = \frac{B}{24}\frac{d_b g \left|\rho_f - \rho_g\right|}{3\rho_f}

whose left hand side is zero at the origin and strictly increasing, so the root is unique and lies
in $[0, T]$ because the bracket factor is at least one. It is solved by a Newton iteration
safeguarded by that bracket, in the same way as the algebraic slip closure of
[LinearWCNSFV2PSlipVelocityFunctorMaterial.md].

## Treatment of the nonlinearity

Both closure sets are evaluated from the previous iterate of $\xi$. Moving the right hand side to
the left, the kernel contributes a coefficient multiplying $\xi$,

!equation
-\frac{1}{3}\frac{D\rho_g}{Dt}
  - \frac{2}{3}\frac{\dot m_g}{\alpha_g}
  - \rho_g \frac{S_{RC}+S_{WE}}{\xi_{old}}

and a remainder $\rho_g S_{TI}$ on the right hand side. The coalescence sinks are non-positive by
construction, so dividing them by $\xi_{old}$ and putting them on the diagonal contributes a
non-negative coefficient there, while the non-negative breakage source is left on the right hand
side. The split is a linearization, not an approximation: at convergence $\xi = \xi_{old}$ and the
pair reproduces the equation exactly.

!alert note
The expansion contribution to the diagonal carries the sign of the material derivative of the
dispersed phase density and is not guaranteed positive. In a strongly compressible transient it can
reduce the diagonal.

## Guards

Four guards are applied, and they are the only departures from the reference.

- $\alpha_g$ is clamped into $[0, \alpha_{g,max}]$ before any use, so the packing factors cannot
  change sign.
- If $\alpha_g = 0$ or $\xi \leq 0$ there is no interface to coalesce or break, and all three
  source terms are set to zero. Returning here rather than flooring $d_b$ matters, because
  $d_b = \psi\alpha_g/\xi$ inverts $\xi$: an additive offset of `libMesh::TOLERANCE`, $10^{-6}$,
  would bias a millimetre-sized particle by one part in $10^{4}$.
- The packing differences $\left(\alpha_{g,max}-\alpha_g\right)$ and
  $\left(\alpha_{g,max}^{1/3}-\alpha_g^{1/3}\right)$ are floored at $10^{-6}$, capping the
  divergence exactly at the packing limit.
- $C_D$, and with it $S_{WE}$, is set to zero when $Re_D \leq 10^{-6}$.

## Verification

Both closure sets are verified against their analytic steady states by the
`interface_area_hibiki_ishii` and `interface_area_ishii_kim` tests. With no flow, a constant
dispersed phase density and no mass transfer, the equation reduces to
$\partial \xi/\partial t = S_{RC}+S_{WE}+S_{TI}$, whose steady state is a balance of coalescence
against breakage. Every term of both models carries the same power of $\xi$, so that balance is a
scalar equation in $d_b$ alone and is therefore a property of the closure formulas rather than of
the discretization. The computed steady states agree with roots obtained independently by a
bracketing solver to twelve significant figures.

!alert note
This object no longer follows [WCNSFV2PInterfaceAreaSourceSink.md]. The nonlinear implementation
uses a different set of source terms, so the two discretizations do not solve the same interfacial
area equation.

!syntax parameters /LinearFVKernels/LinearWCNSFV2PInterfaceAreaSourceSink

!syntax inputs /LinearFVKernels/LinearWCNSFV2PInterfaceAreaSourceSink

!syntax children /LinearFVKernels/LinearWCNSFV2PInterfaceAreaSourceSink
