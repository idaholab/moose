# INSFVMomentumAdvection

This object implements the $\nabla \cdot \left(\rho\vec u \otimes \vec u\right)$ component
terms of the
incompressible Navier Stokes momentum equation. An average or Rhie-Chow
interpolation can be used for the advecting velocity interpolation. An average
or upwind interpolation can be used for the advected quantity, which in this
case is the momentum component $\rho u_i$ where $u_i$ denotes the x, y, or z
component of the velocity in Cartesian coordinates, or the r or z component of
the velocity in RZ coordinates.

The default is to use the same $a$ coefficients when computing the kernel residuals
that strongly affect the solution and when computing the Rhie-Chow
velocity which weakly affects the solution by perturbing the linearly
interpolated velocity as shown in the Rhie-Chow velocity computation shown in
[INSFVRhieChowInterpolator.md], which we reproduce here:

\begin{equation}
\label{rcvel}
\bm{v}_f = \overline{\bm{v}_f} - \overline{\bm{D}_f}\left(\nabla p_f - \overline{\nabla p_f}\right)
\end{equation}

where

!equation
\overline{D_{fi}} = \overline{\frac{V}{a_i}}

where $V$ corresponds to element volumes and $i$ denotes a Cartesian component,
e.g. $x$, $y$, $z$.

However, this default treatment
creates some challenges. It adds additional nonlinearity to the already
nonlinear momentum advection term because of the presence of $\vec{u}\cdot\hat{n}$ in
the $a$ coefficients. This additional nonlinearity may be eliminated by
performing an approximated calculation of $a$. The approximation of $a$
substitutes $\vec{u}\cdot\hat{n}$ with a `characteristic_speed` provided by the
user. This has the effect of converting the default anisotropic,
solution-dependent advection contribution to $a$ to a constant (as long as the
density is constant), isotropic contribution more analogous to the
[INSFVMomentumDiffusion.md] contribution to $a$.

This approximate calculation of the contribution of advection to $a$ may change the amount of velocity
perturbation in [rcvel]. Increasing perturbation may decrease error in the
pressure solution due to decreasing the pressure checkerboard and increase error
in the velocity solution.

!syntax parameters /FVKernels/INSFVMomentumAdvection

!syntax inputs /FVKernels/INSFVMomentumAdvection

!syntax children /FVKernels/INSFVMomentumAdvection
