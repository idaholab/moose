# MFEMComplexVectorPeriodAveragedPostprocessor

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the integral of the period-averaged dot product between two complex vector variables over a
user-specified subdomain of the mesh:

!equation
\langle(k \vec u, \vec v)_\Omega\rangle_T

where $\vec u$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $k$ is an optional scalar coefficient, $\Omega$ is the user-specified mesh subdomain, and the angled brackets denote averaging over a period $T$.

## Example Input File Syntax

!listing mfem/complex/complex_waveguide.i block=Postprocessors

!syntax parameters /Postprocessors/MFEMComplexVectorPeriodAveragedPostprocessor

!syntax inputs /Postprocessors/MFEMComplexVectorPeriodAveragedPostprocessor

!syntax children /Postprocessors/MFEMComplexVectorPeriodAveragedPostprocessor

!if-end!

!else
!include mfem/mfem_warning.md
