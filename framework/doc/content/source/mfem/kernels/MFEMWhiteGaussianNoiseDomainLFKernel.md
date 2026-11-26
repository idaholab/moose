# MFEMWhiteGaussianNoiseDomainLFKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for the linear form

!equation
\langle \dot W, v \rangle \,\,\, \forall v \in V

where $\dot W$ is spatial white Gaussian noise. This is the right hand side of stochastic PDEs
driven by white noise, such as the SPDE approach to sampling Gaussian random fields. After
discretization, the right hand side vector $b$ is a Gaussian random vector $b \sim N(0, M)$
whose covariance is the mass matrix $M$ of the test space. MFEM builds it element by element as
$b = P^T \mathrm{diag}(L_e) w$, where $P$ assembles element vectors into the global vector,
$L_e L_e^T = M_e$ factors the element mass matrix $M_e$, and each entry of $w$ is drawn
independently from $N(0, 1)$.

The random number generator is seeded with [!param](/Kernels/MFEMWhiteGaussianNoiseDomainLFKernel/seed),
offset by the MPI rank. A positive seed gives a reproducible sequence of samples, and the same
sample is drawn each time the linear form is rebuilt. A seed of zero seeds the generator from the
current time.

`createLFIntegrator()` returns an [`mfem::WhiteGaussianNoiseDomainLFIntegrator`](https://docs.mfem.org/html/classmfem_1_1WhiteGaussianNoiseDomainLFIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMWhiteGaussianNoiseDomainLFKernel

!syntax inputs /Kernels/MFEMWhiteGaussianNoiseDomainLFKernel

!syntax children /Kernels/MFEMWhiteGaussianNoiseDomainLFKernel

!if-end!

!else
!include mfem/mfem_warning.md
