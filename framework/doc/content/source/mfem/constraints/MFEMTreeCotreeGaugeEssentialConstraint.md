# MFEMTreeCotreeGaugeEssentialConstraint

!if! function=hasCapability('mfem')

## Overview

Applies a *tree-cotree gauge* to an $H(\mathrm{curl})$ (first-kind Nedelec) variable
$\vec u$. The curl-curl operator has a large null space consisting of the gradients of
$H^1$ functions,

!equation
\vec\nabla \times \left(k \vec\nabla \times \vec u\right) = 0 \quad \text{for } \vec u = \vec\nabla \phi ,

so a problem of the form $(k\,\vec\nabla\times\vec u, \vec\nabla\times\vec v) = (\vec f, \vec v)$ with no
(or only partial) mass term is singular. This is the situation for the magnetic vector
potential $\vec A$ in a magnetodynamic A-formulation: inside a conductor the
$\sigma\,\partial_t\vec A$ term removes the null space, but any surrounding non-conducting
region has no such term.

The constraint removes the null space by building a spanning tree of the mesh 1-skeleton
(its vertices and edges) and strongly fixing the lowest-order edge degree of freedom on
every tree edge to zero. The remaining "cotree" edge DOFs are exactly enough to represent
any curl, so the gauged system is non-singular while the physical solution
$\vec\nabla\times\vec u$ is unchanged.

The forest is grown by a distributed Boruvka pass in which no rank ever holds more than its
own share of the mesh graph. Each edge is keyed on its two endpoint coordinates; vertex
coordinates are copied verbatim from the serial mesh when it is partitioned, so they are
bit-identical on every rank. A distributed sort turns those coordinates into dense global
vertex ids in canonical (lexicographic) order, which gives every edge a weight that depends
only on the geometry and not on the partitioning.

Because those weights are distinct, the minimum spanning forest is *unique*, so the result
is the same as a serial Kruskal pass over the globally sorted edge list. The gauge - and
hence the solution - is therefore independent of the number of MPI ranks and of how the mesh
was partitioned, without the construction ever being replicated.

The `boundary` parameter should list the boundaries on which a tangential Dirichlet
("PEC") condition is applied to the variable. Edges on those boundaries are already fixed
by the boundary condition; they are used to seed the spanning forest so that the interior
gauge stays compatible with the boundary condition instead of over-constraining it.

### Restricting the gauge with `block`

In an A-formulation eddy-current problem the $\sigma\,\partial_t\vec A$ term already fixes
the gauge inside a conductor, so there is no gauge freedom to remove there. Set `block` to
the non-conducting subdomain(s); edges of the excluded (conductor) elements then seed the
spanning forest but are never gauged, while vacuum edges that merely touch the conductor
surface are still gauged, so every free vacuum vertex is reached. Leaving `block` empty
gauges the whole mesh, which is correct only where the curl-curl operator is genuinely
unregularised everywhere (magnetostatics with a divergence-free source).

For the example below, gauging only the vacuum reproduces the solution obtained from a
vanishing artificial regularisation to six significant figures, whereas a whole-mesh gauge
perturbs it by several percent.

For a complex (time-harmonic) variable use
[MFEMComplexTreeCotreeGaugeEssentialConstraint.md], which selects the same degrees of
freedom (the selection is shared via `Moose::MFEM::TreeCotreeGauge`) and fixes both their real
and imaginary parts.

### Notes and limitations

- The gauge acts on the lowest-order (edge) DOFs only. For `fec_order` greater than
  `FIRST` the higher-order gradient modes are not removed.
- Conforming meshes only; the coordinate key assumes vertex coordinates are identical
  across ranks (true for a mesh read from a file, not for periodic meshes).
- Construction is fully distributed in memory, but it is a global graph computation: it
  costs `O(log V)` communication rounds and is rebuilt whenever the mesh or the finite
  element space is refined. The result is cached between solves, so a transient pays for it
  once rather than once per time step.

## Example Input File Syntax

!listing test/tests/mfem/constraints/tree_cotree_gauge_magnetodynamic.i block=Constraints

!syntax parameters /Constraints/MFEMTreeCotreeGaugeEssentialConstraint

!syntax inputs /Constraints/MFEMTreeCotreeGaugeEssentialConstraint

!syntax children /Constraints/MFEMTreeCotreeGaugeEssentialConstraint

!if-end!

!else
!include mfem/mfem_warning.md
