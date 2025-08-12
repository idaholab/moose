# Darcy Problem

## Summary

Solves a 2D mixed Darcy problem. This is a saddle point problem, discretized
using Raviart-Thomas finite elements (velocity $\vec u$) and piecewise
discontinuous polynomials (pressure $p$). This example demonstrates the use of
transposition in the input file for mixed problems with different trial and
test variables. This example is based on [MFEM Example 5](https://mfem.org/examples/).

## Description

This problem solves a mixed Darcy problem with strong form:

\begin{equation}
\begin{split}
k \vec u + \vec \nabla p &= \vec f \\
-\vec \nabla \cdot \vec u &= g \\
p = h \,\,\, &\mathrm{on}\,\, \partial\Omega
\end{split}
\end{equation}

where $k = 1$, $\vec f = \vec 0$, $g = 0$ and $h = e^x \sin(y)$.

In this example, we solve this using the weak form

\begin{equation}
\begin{split}
(k \vec u, \vec v)_\Omega - (p, \vec \nabla \cdot \vec v)_\Omega + (h, \vec v \cdot \hat n)_{\partial\Omega}
&= 0 \,\,\, \forall \vec v \in V \\
- (\vec \nabla \cdot \vec u, w)_\Omega &= 0 \,\,\, \forall w \in W
\end{split}
\end{equation}

where

\begin{equation}
\begin{split}
\vec u, \vec v &\in H(\mathrm{div})(\Omega) \\
p, w &\in L^2(\Omega)
\end{split}
\end{equation}

## Example File

!listing test/tests/mfem/kernels/darcy.i
