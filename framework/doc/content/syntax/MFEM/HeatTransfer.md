# Transient Heat Transfer

## Summary

Solves a transient heat conduction problem with a
  boundary parameterized by a heat transfer coefficient that exchanges heat with a thermal
  reservoir.

## Description

This problem solves the transient heat equation with strong form:

\begin{equation}
\begin{split}
\rho c_p \frac{\partial T}{\partial t} - \vec \nabla \cdot \left(k\vec\nabla T\right) = 0 \,\,\,&\mathrm{on}\,\, \Omega \\
k \vec \nabla T \cdot \hat n = h_\mathrm{htc}\left(T-T_\mathrm{inf}\right) \,\,\, &\mathrm{on}\,\, \Gamma_\mathrm{htc} \\
T = T_0 \,\,\, &\mathrm{on}\,\, \Gamma_\mathrm{D}
\end{split}
\end{equation}

where $T_0 = 1.0$, $T_\mathrm{inf} = 0.5$, $\rho c_p = 1.0$, and $h_\mathrm{htc} = 5.0$, subject to
the initial condition $T(t=0)=0.0$.

In this example, we solve this using the weak form

!equation
\left(\rho c_p \frac{\partial T}{\partial t}, v\right)_\Omega + (k \vec \nabla T, \vec \nabla v)_\Omega
- (k \vec \nabla T \cdot \hat n, v)_{\partial \Omega}
= 0 \,\,\, \forall v \in V

where

\begin{equation}
\begin{split}
T \in H^1(\Omega) &: T = T_0 \,\,\, \mathrm{on}\,\, \Gamma_\mathrm{D} \\
v \in H^1(\Omega) &: v = 0 \,\,\, \mathrm{on}\,\, \Gamma_\mathrm{D}
\end{split}
\end{equation}

## Example File

!listing test/tests/mfem/kernels/heattransfer.i
