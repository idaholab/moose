# MFEMComplexTreeCotreeGaugeEssentialConstraint

!if! function=hasCapability('mfem')

## Overview

Complex (time-harmonic) counterpart of [MFEMTreeCotreeGaugeEssentialConstraint.md].
It applies a *tree-cotree gauge* to a complex $H(\mathrm{curl})$ (first-kind Nedelec)
variable, for example the magnetic vector potential $\vec A$ in a frequency-domain
A-formulation

!equation
\vec\nabla\times\left(\nu\,\vec\nabla\times\vec A\right) + i\omega\sigma\vec A = \vec J .

The $i\omega\sigma\vec A$ term removes the gradient null space of the curl-curl operator
inside a conductor, but any surrounding non-conducting region has no such term and the
discrete operator is singular there.

The set of edge degrees of freedom to fix is computed exactly as in the real case - a
distributed Boruvka pass grows the unique minimum spanning forest of the mesh graph under a
canonical, geometry-derived edge order, so the gauge is independent of the MPI partitioning
(the implementation is shared through `Moose::MFEM::TreeCotreeGauge`). Here the
**real and imaginary parts** of each gauged edge dof are both strongly set to zero.

The `boundary` and `block` parameters behave as for the real constraint: `boundary` lists
the boundaries carrying a tangential Dirichlet condition on the variable, and `block`
restricts the gauge to the given (non-conducting) subdomains, with the complementary
subdomains seeding the forest but never being gauged.

## Example Input File Syntax

!listing test/tests/mfem/constraints/tree_cotree_gauge_time_harmonic.i block=Constraints

!syntax parameters /Constraints/MFEMComplexTreeCotreeGaugeEssentialConstraint

!syntax inputs /Constraints/MFEMComplexTreeCotreeGaugeEssentialConstraint

!syntax children /Constraints/MFEMComplexTreeCotreeGaugeEssentialConstraint

!if-end!

!else
!include mfem/mfem_warning.md
