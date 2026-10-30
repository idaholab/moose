# MFEMTreeCotreeGaugeEssentialConstraint

!if! function=hasCapability('mfem')

## Overview

Applies a *tree-cotree gauge* to an $H(\mathrm{curl})$ (first-kind Nedelec) variable
$\vec u$. The curl-curl operator has a large null space consisting of the gradients of
$H^1$ functions,

!equation
\vec\nabla \times \left(k \vec\nabla \times \vec u\right) = 0 \quad \text{for } \vec u = \vec\nabla \phi ,

so a problem of the form $(k\,\vec\nabla\times\vec u, \vec\nabla\times\vec v) = (\vec f, \vec v)$ with no
(or only partial) mass term is singular. Whenever a mass-like term is absent over part or all
of the domain, that part of the operator retains the null space and the discrete system
cannot be solved without either an artificial regularization or a gauge.

The constraint removes the null space by building a spanning tree of the mesh 1-skeleton
(its vertices and edges) and strongly fixing the lowest-order edge degree of freedom on
every tree edge to zero. The remaining "cotree" edge DOFs are exactly enough to represent
any curl, so the gauged system is non-singular while the physical solution
$\vec\nabla\times\vec u$ is unchanged.

The forest is grown by a distributed Boruvka pass in which no rank ever holds more than its
own share of the mesh graph. Each edge is keyed on its two endpoint coordinates, read from
the mesh nodes, which are bit-identical on every rank sharing a vertex. A distributed sort turns those coordinates into dense global
vertex ids in canonical (lexicographic) order, which gives every edge a weight that depends
only on the geometry and not on the partitioning.

Because those weights are distinct, the minimum spanning forest is *unique*, so the result
is the same as a serial Kruskal pass over the globally sorted edge list. The gauge - and
hence the solution - is therefore independent of the number of MPI ranks and of how the mesh
was partitioned, without the construction ever being replicated.

### Seeding the forest from essential conditions

Edges whose degree of freedom is already fixed, by an essential (tangential Dirichlet)
boundary condition or by another essential constraint on the variable, seed the spanning
forest, so that the gauge stays compatible with those conditions instead of
over-constraining them. The constraint takes these degrees of freedom from the equation
system, which applies the gauge after every other essential constraint on the variable.

### Restricting the gauge with `block`

Where another term of the weak form already removes the null space, there is no gauge
freedom left to fix and gauging anyway would over-constrain the solution. Set `block` to the
subdomain(s) that do need gauging; all edges of the excluded elements, including those
at the interface between gauged and non-gauged subdomains, then seed the spanning
forest but are never gauged. Leaving `block` empty gauges the whole
mesh, which is correct only when the operator is genuinely unregularised everywhere.

Getting this wrong is not a subtle effect: in the example below, gauging only the
unregularised subdomain reproduces the solution obtained from a vanishing artificial
regularization to six significant figures, whereas a whole-mesh gauge perturbs it by several
percent.

For a complex variable use
[MFEMComplexTreeCotreeGaugeEssentialConstraint.md], which selects the same degrees of
freedom (the selection is shared via `Moose::MFEM::TreeCotreeGauge`) and fixes both their real
and imaginary parts.

### Notes and limitations

- The gauge acts on the lowest-order (edge) DOFs only. For `fec_order` greater than
  `FIRST` the higher-order gradient modes are not removed.
- The variable must be an $H(\mathrm{curl})$ (Nedelec) variable.
- Conforming meshes only. Meshes with discontinuous nodes, such as periodic meshes, are
  rejected, because their vertices cannot be identified by coordinates.
- Construction is fully distributed in memory, but it is a global graph computation: for
  a mesh with `V` vertices, it costs `O(log V)` communication rounds and is rebuilt whenever the mesh or the finite
  element space is refined. The result is cached between solves, so a transient pays for it
  once rather than once per time step.

## Example: a magnetodynamic A-formulation

The parameters above are formulation-independent; this is one concrete case. For the magnetic
vector potential $\vec A$ in an eddy-current A-formulation, the conducting region carries a
$\sigma\,\partial_t\vec A$ term that removes the null space, but a surrounding non-conducting
region has no such term and is singular. Setting `block` to the non-conducting subdomain(s)
gauges exactly the region that needs it, while the boundaries carrying the tangential
Dirichlet condition on $\vec A$ (a perfect electric conductor, or "PEC", boundary in that
terminology) seed the forest.

## Example Input File Syntax

!listing test/tests/mfem/constraints/tree_cotree_gauge_magnetodynamic.i block=Constraints

!syntax parameters /Constraints/MFEMTreeCotreeGaugeEssentialConstraint

!syntax inputs /Constraints/MFEMTreeCotreeGaugeEssentialConstraint

!syntax children /Constraints/MFEMTreeCotreeGaugeEssentialConstraint

!if-end!

!else
!include mfem/mfem_warning.md
