# MFEMComplexScalarEssentialConstraint

!if! function=hasCapability('mfem')

## Overview

Complex (time-harmonic) counterpart of [MFEMScalarEssentialConstraint.md]. It strongly
constrains the real and imaginary parts of a complex scalar variable within the subdomain(s)
given by `block`, projecting them onto the `coefficient_real` and `coefficient_imag`
coefficients respectively. The true degrees of freedom in the constrained subdomain are
added to the essential-DOF list, so both components of the discrete system row are
eliminated in favour of the projected values.

## Example Input File Syntax

!listing test/tests/mfem/constraints/complex_subdomain_constraint_source.i block=Constraints

!syntax parameters /Constraints/MFEMComplexScalarEssentialConstraint

!syntax inputs /Constraints/MFEMComplexScalarEssentialConstraint

!syntax children /Constraints/MFEMComplexScalarEssentialConstraint

!if-end!

!else
!include mfem/mfem_warning.md
