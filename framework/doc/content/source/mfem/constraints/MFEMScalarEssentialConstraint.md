# MFEMScalarEssentialConstraint

!if! function=hasCapability('mfem')

## Overview

Strongly constrains a scalar variable within the subdomain(s) given by `block`, projecting
it onto the `coefficient` coefficient. The true degrees of freedom in the constrained
subdomain are added to the essential-DOF list, so the corresponding rows of the discrete
system are eliminated in favour of the projected values. This is the volumetric analogue of
an [MFEMScalarDirichletBC.md] and is useful for imposing a known interior value (for
example fixing a field inside a solid region of the mesh).

For a complex (time-harmonic) variable use [MFEMComplexScalarEssentialConstraint.md].

## Example Input File Syntax

!listing test/tests/mfem/constraints/subdomain_constraint_source.i block=Constraints

!syntax parameters /Constraints/MFEMScalarEssentialConstraint

!syntax inputs /Constraints/MFEMScalarEssentialConstraint

!syntax children /Constraints/MFEMScalarEssentialConstraint

!if-end!

!else
!include mfem/mfem_warning.md
