# MFEMVectorEssentialConstraint

!if! function=hasCapability('mfem')

## Overview

Strongly constrains a vector variable within the subdomain(s) given by `block`, projecting it
onto the `vector_coefficient` coefficient. The true degrees of freedom in the constrained
subdomain are added to the essential-DOF list, so the corresponding rows of the discrete
system are eliminated in favour of the projected values. This is the vector analogue of
[MFEMScalarEssentialConstraint.md] and works for vector H1 as well as H(curl) and H(div)
function spaces.

For a complex (time-harmonic) variable use [MFEMComplexVectorEssentialConstraint.md].

## Example Input File Syntax

!listing test/tests/mfem/constraints/vector_subdomain_constraint_source.i block=Constraints

!syntax parameters /Constraints/MFEMVectorEssentialConstraint

!syntax inputs /Constraints/MFEMVectorEssentialConstraint

!syntax children /Constraints/MFEMVectorEssentialConstraint

!if-end!

!else
!include mfem/mfem_warning.md
