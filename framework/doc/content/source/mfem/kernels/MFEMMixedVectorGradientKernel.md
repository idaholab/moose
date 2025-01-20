# MFEMMixedVectorGradientKernel

## Summary

!syntax description /Kernels/MFEMMixedVectorGradientKernel

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec \nabla u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $u \in H^1$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, and $k$ is a scalar
coefficient.

This term arises from the weak form of the gradient operator

!equation
k \vec\nabla u

!syntax parameters /Kernels/MFEMMixedVectorGradientKernel

!syntax inputs /Kernels/MFEMMixedVectorGradientKernel

!syntax children /Kernels/MFEMMixedVectorGradientKernel
