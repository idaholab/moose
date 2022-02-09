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

!syntax parameters /UserObjects/INSFVRhieChowInterpolator

!syntax inputs /UserObjects/INSFVRhieChowInterpolator

!syntax children /UserObjects/INSFVRhieChowInterpolator
