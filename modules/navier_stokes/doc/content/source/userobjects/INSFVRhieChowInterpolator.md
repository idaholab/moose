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
  - $\bm{B}$ coefficients: these correspond to the body forces acting on the momentum
    equation
  - All momentum residual objects, other than objects that represent the
    pressure gradient term, should contribute to $a$ or $\bm{B}$ coefficient
    data

- Takes the $\bm{B}$ (body force) data and performs successive
  interpolation-reconstruction-interpolation operations necessary to obtain the
  single, double, and triple overbar terms necessary to implement the body force
  interpolation scheme outlined in equations 15.202 and 15.214 of
  [!citep](moukalled2016finite), equations reproduced below in [rcbody] and
  [rcvel] where $C$ subscripts denote the current cell center, $f$ subscripts
  indicate a cell face, $F$ subscripts denote the current cell neighbors,
  $\bm{v}$ is the velocity field, $\bm{b}$ denotes terms like old parts of time
  derivative terms and prescribed boundary fluxes, $V$ is the current cell
  volume, and $\bm{D}$ corresponding to a three-vector with components equal to
  $V/a$.
- Computes the Rhie-Chow velocity according to [rcvel] when requested by
  advection kernels or postprocessors

\begin{equation}
\label{rcbody}
a_C \bm{v}_C = -\sum_{F~NB(C)} a_F \bm{v}_F + \bm{b}_C -V_C\left(\nabla
p\right)_C + V_C \overline{\overline{\bm{B}}}
\end{equation}

\begin{equation}
\label{rcvel}
\bm{v}_f = \overline{\bm{v}_f} - \overline{\bm{D}_f}\left(\nabla p_f - \overline{\nabla p_f}\right) + \overline{\bm{D}_f}\left(\overline{\bm{B}_f} - \overline{\overline{\overline{\bm{B}_f}}}\right)
\end{equation}

!syntax parameters /UserObjects/INSFVRhieChowInterpolator

!syntax inputs /UserObjects/INSFVRhieChowInterpolator

!syntax children /UserObjects/INSFVRhieChowInterpolator
