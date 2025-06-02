# Grad-div Problem

## Summary

Solves a diffusion problem for a vector field on a cuboid domain, discretized
using $H(\mathrm{div})$ conforming Raviart-Thomas elements. This example is
based on [MFEM Example 4](https://mfem.org/examples/) and is relevant for
solving Maxwell's equations using potentials without the Coulomb gauge.


## Description

This problem solves a grad-div equation with strong form:

\begin{equation}
\begin{split}
-\vec\nabla \left( \alpha \vec\nabla \cdot \vec F \right) + \beta \vec F = \vec f \,\,\,&\mathrm{on}\,\, \Omega \\
\vec F \cdot \hat n= \vec g \,\,\, &\mathrm{on}\,\, \partial\Omega
\end{split}
\end{equation}

where

\begin{equation}
\vec g = \begin{pmatrix}
    \cos(k x)\sin(k y)\\
    \cos(k y)\sin(k x)\\
    0
\end{pmatrix},\,\,\,
\vec f = \left( \beta + 2 \alpha k^2 \right) \vec g
\end{equation}

In this example, we solve this using the weak form

!equation
(\alpha \vec\nabla \cdot \vec F , \vec\nabla \cdot \vec v)_\Omega + (\beta \vec F, \vec v)_\Omega
= (\vec f, \vec v)_\Omega \,\,\, \forall \vec v \in V

where

\begin{equation}
\begin{split}
\vec F \in H(\mathrm{div})(\Omega) &: \vec F \cdot \hat n= \vec g \,\,\, \mathrm{on}\,\, \partial\Omega \\
\vec v \in H(\mathrm{div})(\Omega) &: \vec v \cdot \hat n= \vec 0 \,\,\, \mathrm{on}\,\, \partial\Omega
\end{split}
\end{equation}

## Example File

!listing test/tests/mfem/kernels/graddiv.i
