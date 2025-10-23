# Transient Heat Transfer (Mixed Form)

## Summary

Solves a transient heat conduction problem using a mixed weak form, representing temperature on piecewise constant $L^2$ conforming finite elements, and heat fluxes on $H(\mathrm{div})$ conforming Raviart-Thomas elements.

## Description

This problem solves the transient heat equation with strong form:

\begin{equation}
\begin{split}
\rho c_p \frac{\partial T}{\partial t} - \vec \nabla \cdot \left(k\vec\nabla T\right) = 0 \,\,\,&\mathrm{on}\,\, \Omega \\
k \vec \nabla T \cdot \hat n = 0 \,\,\, &\mathrm{on}\,\, \Gamma_\mathrm{h} \\
T = T_0 \,\,\, &\mathrm{on}\,\, \Gamma_\mathrm{T}
\end{split}
\end{equation}

where $\rho c_p = 1.0$ and $k^{-1} = 1.0$, subject to the initial condition $T(t=0)=0.0$.

In this example, we solve this by introducing the heat flux $\vec h = -k \vec \nabla T$ to generate
the mixed weak form of the transient heat equation

\begin{equation}
\begin{split}
\left(\rho c_p \frac{\partial T}{\partial t}, T'\right)_\Omega + \left(\vec \nabla \cdot \vec h, T'\right)_\Omega
= 0 \,\,\, \forall T' \in V_T \\
\left(k^{-1} \vec h, \vec h'\right)_\Omega - (T, \vec \nabla \cdot \vec h')_\Omega
= (T , \vec h' \cdot \hat n)_{\partial \Omega} \,\,\, \forall \vec h' \in V_h
\end{split}
\end{equation}

where

\begin{equation}
\begin{split}
T, T' &\in L_2(\Omega)\\
\vec h, \vec h' &\in H(\mathrm{div})(\Omega) : \vec h \cdot \hat n = 0 \,\,\, \mathrm{on}\,\, \Gamma_\mathrm{h}
\end{split}
\end{equation}

and the polynomial order of the FEs used to represent $\vec h$ and $\vec h'$ is one order higher than $T$ and $T'$.

Notably, in contrast to the primal weak form for the transient heat equation solved [here](syntax/MFEM/HeatTransfer.md), the heat flux on $\Gamma_\mathrm{h}$ is strongly imposed via a Dirichlet condition, and temperatures on boundaries $\Gamma_\mathrm{T}$ are imposed weakly via boundary integrals.

## Example File

!listing test/tests/mfem/kernels/mixed_heattransfer.i
