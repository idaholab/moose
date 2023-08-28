# INSFVRhieChowInterpolator

!syntax description /UserObjects/INSFVRhieChowInterpolator

## Overview

This object coordinates everything about an incompressible Navier-Stokes finite
volume (INSFV) simulation related to Rhie-Chow. This object performs the
following activities

- Loops through all momentum residual objects and gathers information related to
- $a$ coefficients: these are the coefficients that multiply the
  on-diagonal/current-cell velocity solution in a linearized writing of the
  momentum equation

- Computes the Rhie-Chow velocity according to [rcvel] when requested by
  advection kernels or postprocessors

\begin{equation}
\label{rcvel}
\bm{v}_f = \overline{\bm{v}_f} - \overline{\bm{D}_f}\left(\nabla p_f - \overline{\nabla p_f}\right)
\end{equation}

The default is to use the same $a$ coefficients when computing the kernel residuals
that strongly affect the solution and when computing the Rhie-Chow
velocity which weakly affects the solution by perturbing the linearly
interpolated velocity as shown in [rcvel]. However, this default treatment
creates some challenges. It adds additional nonlinearity to the already
nonlinear momentum advection term because of the presence of $\vec{v}\cdot n$ in
the $a$ coefficients. This additional nonlinearity may be eliminated by
performing an approximated calculation of $a$. The approximation of $a$
considers only the impacts of advection and diffusion/viscosity, the effects
included in calculation of the Reynolds number. We outline this approximate
computation below.

Recall from [!citep](moukalled2016) that $D = V / a$. And $a$ represents the coefficients multiplying the
velocity components--including volume integration--in a linearized casting of the momentum
equations. So both numerator ($V$) and denominator ($a$) contain units of volume. We can imagine
dividing both numerator and denominator by volume then, and so $D$ should have the inverse of
the units matching the linearized coefficient matrix of the *strong form* (not volume
integrated) of the momentum equations, e.g. if we consider the advection term:

!equation
\nabla \cdot \rho uu

then D should have the inverse of the units correponding to

!equation
\nabla \cdot \rho u

and similarly the inverse of the units of

!equation
\nabla \cdot \mu \nabla

So, we can perform an approximate compution of $D$ with the following formula

!equation
D_f = \frac{1}{1/L * \rho * s + 1/L * \mu * 1/L}

where $s$ represents some approximate representation of the velocity such that
we can drop the system nonlinearity introducted by the default computation of
$a$/$D$. Multiplying both numerator and denominator by $L^2$ yields:

!equation
D_f = \frac{L^2}{L * \rho * s + \mu}

For $L$ we use the norm of the distance between the element centroids on either
side of the face $f$. For $s$, we ask that the user provide it through the
`characteristic_speed` parameter. This parameter should only be provided when
the approximate treatment is requested by setting
`approximate_rhie_chow_coefficients = true` in the input file.

This approximate calculation of $a$/$D$ may change the amount of velocity
perturbation in [rcvel]. Increasing perturbation may decrease error in the
pressure solution due to decreasing the pressure checkerboard and increase error
in the velocity solution.


### Accessing 'a' coefficient data in nonstandard locations

If you ever come across an error like

```
Attempted access into CellCenteredMapFunctor 'a' with a key that does not yet
exist in the map. Make sure to fill your CellCenteredMapFunctor for all elements
you will attempt to access later.
```

then it means an object you're using in your input file is attempting to access
Rhie-Chow data in unexpected locations. This can potentially be remedied by
setting the parameter `pull_all_nonlocal_a = true`. This will tell all processes
to pull 'a' coefficient data for all elements they have access to (which may not
be all the elements in the mesh if the mesh is distributed) from the processes
that own the 'a' coefficient data.

!syntax parameters /UserObjects/INSFVRhieChowInterpolator

!syntax inputs /UserObjects/INSFVRhieChowInterpolator

!syntax children /UserObjects/INSFVRhieChowInterpolator
