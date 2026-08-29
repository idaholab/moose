# MFEMComplexVectorEssentialConstraint

!if! function=hasCapability('mfem')

## Overview

Complex (time-harmonic) counterpart of [MFEMVectorEssentialConstraint.md]. It strongly
constrains the real and imaginary parts of a complex vector variable within the subdomain(s)
given by `block`, projecting them onto the `vector_coefficient_real` and
`vector_coefficient_imag` coefficients respectively. The true degrees of freedom in the
constrained subdomain are added to the essential-DOF list, so both components of the discrete
system row are eliminated in favour of the projected values. It works for vector H1 as well
as H(curl) and H(div) function spaces.

## Example Input File Syntax

!listing test/tests/mfem/constraints/complex_vector_subdomain_constraint_source.i block=Constraints

!syntax parameters /Constraints/MFEMComplexVectorEssentialConstraint

!syntax inputs /Constraints/MFEMComplexVectorEssentialConstraint

!syntax children /Constraints/MFEMComplexVectorEssentialConstraint

!if-end!

!else
!include mfem/mfem_warning.md
