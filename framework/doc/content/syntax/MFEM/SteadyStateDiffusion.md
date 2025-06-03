# Steady State Diffusion

## Summary

Solves a steady state diffusion problem for the
  temperature profile across a mug with fixed temperatures on two boundaries.

## Description

This problem solves the steady state heat equation with strong form:

\begin{equation}
\begin{split}
\vec \nabla \cdot \left(k\vec\nabla T\right) = 0 \,\,\,&\mathrm{on}\,\, \Omega \\
T = T_0 \,\,\, &\mathrm{on}\,\, \Gamma_\mathrm{D} \\
k \vec \nabla T \cdot \hat n = 0 \,\,\, &\mathrm{on}\,\, \partial \Omega \backslash \Gamma_\mathrm{D}
\end{split}
\end{equation}

where $k = 1.0$, $T_0 = 1.0$ on the base of the mug and $T_0 = 0.0$ at the top of the mug.

In this example, we solve this using the weak form

!equation
(k \vec \nabla T, \vec \nabla v)_\Omega
= 0 \,\,\, \forall v \in V

where

\begin{equation}
\begin{split}
T \in H^1(\Omega) &: T = T_0 \,\,\, \mathrm{on}\,\, \Gamma_\mathrm{D} \\
v \in H^1(\Omega) &: v = 0 \,\,\, \mathrm{on}\,\, \Gamma_\mathrm{D}
\end{split}
\end{equation}

## Example File

!listing test/tests/mfem/kernels/diffusion.i