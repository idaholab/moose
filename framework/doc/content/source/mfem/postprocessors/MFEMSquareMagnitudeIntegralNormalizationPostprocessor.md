# MFEMSquareMagnitudeIntegralNormalizationPostprocessor

!if! function=hasCapability('mfem')

## Overview

Postprocessor that computes the L2 norm of a scalar variable over the domain $\Omega$ (or a
user-specified subdomain) and writes the normalized variable to an auxiliary gridfunction. The
input variable is left unchanged.

For a real variable $u \in H^1(\Omega)$ or $L^2(\Omega)$ the normalization constant is

!equation
c = \sqrt{\int_\Omega u^2 \, \mathrm{d}\Omega}.

For a complex variable $u = u_r + i\, u_i$ the square magnitude is taken as $u$ multiplied by its
complex conjugate, giving

!equation
c = \sqrt{\int_\Omega |u|^2 \, \mathrm{d}\Omega} = \sqrt{\int_\Omega \left( u_r^2 + u_i^2 \right) \mathrm{d}\Omega}.

The auxiliary variable is set to $u / c$, which satisfies $\lVert u_{\text{aux}} \rVert_{L^2} = 1$.
The postprocessor value returned by this object is the normalization constant $c$.

This postprocessor accepts both real and complex scalar variables; the appropriate integration
path is selected automatically at construction time based on which gridfunction container the named
variable resides in. The auxiliary variable must share the same finite element space as the input
variable.

The integration domain can be restricted to one or more mesh subdomains using the
[!param](/Postprocessors/MFEMSquareMagnitudeIntegralNormalizationPostprocessor/block) parameter.

!syntax parameters /Postprocessors/MFEMSquareMagnitudeIntegralNormalizationPostprocessor

!syntax inputs /Postprocessors/MFEMSquareMagnitudeIntegralNormalizationPostprocessor

!syntax children /Postprocessors/MFEMSquareMagnitudeIntegralNormalizationPostprocessor

!if-end!

!else
!include mfem/mfem_warning.md
