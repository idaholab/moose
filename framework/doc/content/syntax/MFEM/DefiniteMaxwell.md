# Definite Maxwell Problem

## Summary

Solves a 3D electromagnetic diffusion problem for
  the electric field on a cube missing an octant, discretized using $H(\mathrm{curl})$ conforming Nédélec elements. This example
  is based on [MFEM Example 3](https://mfem.org/examples/).


## Description

This problem solves a definite Maxwell equation with strong form:

\begin{equation}
\begin{split}
\vec\nabla \times \vec\nabla \times \vec E + \vec E = \vec f \,\,\,&\mathrm{on}\,\, \Omega \\
\vec E \times \hat n= \vec g \,\,\, &\mathrm{on}\,\, \partial\Omega
\end{split}
\end{equation}

where

\begin{equation}
\vec f = \begin{pmatrix}
    (1+\pi^2)\sin(\pi y)\\
    (1+\pi^2)\sin(\pi z)\\
    (1+\pi^2)\sin(\pi x)
\end{pmatrix},\,\,\,
\vec g = \begin{pmatrix}
    \sin(\pi y)\\
    \sin(\pi z)\\
    \sin(\pi x)
\end{pmatrix}
\end{equation}

In this example, we solve this using the weak form

!equation
(\vec\nabla \times \vec E, \vec\nabla \times \vec v)_\Omega + (\vec E, \vec v)_\Omega
= (\vec f, \vec v)_\Omega \,\,\, \forall \vec v \in V

where

\begin{equation}
\begin{split}
\vec E \in H(\mathrm{curl})(\Omega) &: \vec E \times \hat n= \vec g \,\,\, \mathrm{on}\,\, \partial\Omega \\
\vec v \in H(\mathrm{curl})(\Omega) &: \vec v \times \hat n= \vec 0 \,\,\, \mathrm{on}\,\, \partial\Omega
\end{split}
\end{equation}

## Example File

!listing test/tests/mfem/kernels/curlcurl.i
