# MFEMLinearElasticityKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMLinearElasticityKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(\sigma_{ij}, \partial_j v_i)_\Omega \,\,\, \forall v_i \in V

where $v_i \in H^1$, and $\sigma$ is the stress tensor of a material with an isotropic stress/strain
relation, with components given by

!equation
\sigma_{ij} \equiv C_{ijkl} \varepsilon_{kl} = \lambda \delta_{ij} \varepsilon_{kk} + 2 \mu \varepsilon_{ij}

and

!equation
\varepsilon_{ij} = \frac{1}{2} \left(\partial_i u_j + \partial_j u_i\right)

noting that the Einstein summation convention has been used throughout.

The two material-dependent Lamé parameters $\lambda$ and $\mu$ can be expressed in terms of the
material Young's modulus $E$ and the Poisson ratio $\nu$ using

\begin{equation}
\begin{split}
\lambda &= \frac{E\nu}{(1-2\nu)(1+\nu)} \\
\mu &= \frac{E}{2(1+\nu)}
\end{split}
\end{equation}

## Example Input File Syntax

!listing mfem/kernels/linearelasticity.i

!syntax parameters /Kernels/MFEMLinearElasticityKernel

!syntax inputs /Kernels/MFEMLinearElasticityKernel

!syntax children /Kernels/MFEMLinearElasticityKernel

!if-end!

!else
!include mfem/mfem_warning.md
