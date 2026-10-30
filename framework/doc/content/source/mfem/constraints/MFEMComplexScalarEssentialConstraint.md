# MFEMComplexScalarEssentialConstraint

!if! function=hasCapability('mfem')

## Overview

Complex counterpart of [MFEMScalarEssentialConstraint.md]. It strongly constrains the real and
imaginary parts of a complex scalar variable within the subdomain(s) given by `block` to the
projections of the `coefficient_real` and `coefficient_imag` coefficients, respectively, onto
the variable's finite element space. The true degrees of freedom in the constrained subdomain
are added to the essential-DOF list, so both components of the discrete system row are
eliminated in favor of the projected values.

## Example Input File Syntax

!listing test/tests/mfem/constraints/complex_subdomain_constraint_source.i block=Constraints

!syntax parameters /Constraints/MFEMComplexScalarEssentialConstraint

!syntax inputs /Constraints/MFEMComplexScalarEssentialConstraint

!syntax children /Constraints/MFEMComplexScalarEssentialConstraint

!if-end!

!else
!include mfem/mfem_warning.md
