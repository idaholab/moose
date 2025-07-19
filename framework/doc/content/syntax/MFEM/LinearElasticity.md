# Linear Elasticity Problem

## Summary

Solves a 3D linear elasticity problem for
the deformation of a multi-material cantilever beam. This example
is based on [MFEM Example 2](https://mfem.org/examples/).


## Description

This problem solves a linear elasticity problem for small deformations with strong form:

\begin{equation}
\begin{split}
-\partial_j \sigma_{ij} =0
\,\,\,&\mathrm{on}\,\, \Omega \\
u_i = 0 \,\,\, &\mathrm{on}\,\, \Gamma_1 \\
\sigma_{ij} \hat n_j= f_i \,\,\, &\mathrm{on}\,\, \Gamma_2
\end{split}
\end{equation}

where Einstein notation for summation over repeated indices has been used, and the pull-down force
$\vec f = f_i \hat e_i$ is given by

\begin{equation}
\vec f = \begin{pmatrix}
    0.0\\
    0.0\\
    -0.01
\end{pmatrix}
\end{equation}

In this example,
the stress/strain relation is taken to be isotropic, such that

\begin{equation}
\begin{split}
\sigma_{ij} \equiv C_{ijkl} \varepsilon_{kl} = \lambda \delta_{ij} \varepsilon_{kk} + 2 \mu \varepsilon_{ij} \\
\varepsilon_{ij} = \frac{1}{2} \left(\partial_i u_j + \partial_j u_i\right)
\end{split}
\end{equation}

In this example, we solve this using the weak form

\begin{equation}
(\sigma_{ij}, \partial_j v_i)_\Omega - (\sigma_{ij} \hat n_j, v_i)_{\partial \Omega}
= 0 \,\,\,\, \forall v_i \in V
\end{equation}

where

\begin{equation}
v_i \in H^1(\Omega) : v_i = 0 \,\,\, \mathrm{on} \,\, \Gamma_1
\end{equation}

## Material Parameters

In this example, the cantilever beam is comprised of two materials; a stiffer material on the block
with domain attribute 1, and a more flexible material defined on the block with domain attribute 2.
These materials are parametrised with different  values for the Lamé parameters, $\lambda$ and
$\mu$, on the two domains.

The two Lamé parameters can be expressed in terms of the Young's modulus $E$ and the Poisson ratio $\nu$ in each material, using

\begin{equation}
\begin{split}
\lambda &= \frac{E\nu}{(1-2\nu)(1+\nu)} \\
\mu &= \frac{E}{2(1+\nu)}
\end{split}
\end{equation}

## Example File

!listing test/tests/mfem/kernels/linearelasticity.i
