# AutomaticMortarGeneration

The `AutomaticMortarGeneration` class is used to define mortar segment meshes to properly integrate discontinuities caused by normal projections of non-matching, faceted meshes.
The mortar segment mesh is always replicated when the parent mesh is allowed to displace, otherwise
its distribution type (replicated/distributed) is determined by the type of the parent mesh. The
reason the mortar mesh is replicated during displacement is that we always build the mortar mesh by
looping over all active elements associated with the secondary and primary interface, not just
active local elements. When displacement is occurring we cannot know *a priori* what elements will
project onto each other, so absent that knowledge we keep copies of all interface elements on each
process. In the future we may eliminate this replication and implement communication patterns that
occur during each residual and Jacobian evaluation to pull remote elements. This would improve
memory scalability. Even for the current implementation where the interface of the parent mesh is
replicated, one could legitimately ask why the mortar mesh needs to be so. The answer is that
`MortarNodalAuxKernel` derived classes require access to all mortar segment elements, local or not,
that have support from the shape function associated with a given node. In the future we may add
ghosting functors to the mortar segment mesh, which would allow us to delete unnecessary non-local
elements.

## 2D

Generation of the 2D mortar segment mesh is outlined in [!cite](osti_1468630) and follows the
nodal-normal projection construction of [!cite](yang2005two). Yang et al. define a continuous
secondary normal field by cross-weighting the normals of the adjacent edges. Specifically, their
unnormalized normal at node $A$ is $\hat{\boldsymbol{n}}_A =
l_2\boldsymbol{n}_{A1} + l_1\boldsymbol{n}_{A2}$: the normal of each edge is weighted by the
length of the other edge. They use this field both to project secondary nodes onto the primary
interface and to project primary nodes onto the secondary interface. The secondary interface
elements are split at the projected primary nodes to form the mortar segment mesh elements.

MOOSE deviates from the nodal-normal weighting in [!cite](yang2005two): rather than explicitly
cross-weighting adjacent normals, MOOSE weights each normal by the nodal `JxW` contribution from
that same edge, obtained with a `QNodal` quadrature rule. For straight first-order edges this `JxW`
is proportional to the length of the same edge, so the two definitions differ when adjacent edge
lengths differ. For curved or higher-order edges the MOOSE weight is additionally a local nodal
quadrature/Jacobian contribution rather than a complete-edge length.

## 3D

Generation of mortar segment meshes in 3D is more challenging and various approaches exist. In MOOSE we follow the approach suggested in [!cite](puso20043d), defining mortar segments on local linearizations of the secondary interface. When the secondary interface is composed entirely of TRI3 faces (which are already linear), generating the mortar segment mesh reduces to projecting the primary face elements onto secondary face elements (along the secondary face normal) and performing a polygon clipping (and subsequent triangularization). The mortar segment mesh is therefore simply a sub-mesh of the secondary mesh in this case.

The definition is more delicate for quadrilateral faces and second order geometries:

### QUAD4 faces

While first-order, QUAD4 faces are (in general) not linear. The 'twisting' or 'potato-chipping' of QUAD4 elements complicates the simple projection and polygon clipping defined for TRI3 faces. To circumvent this problem, mortar segment meshes are defined on local linearizations of QUAD4 elements (see below). The linearization of QUAD4 face elements allows the same polygon clipping algorithm used for TRI3 face elements, but the mortar segment mesh elements produced do not coincide with the secondary mesh and the mortar segment mesh is disconnected between secondary elements.

!media media/framework/constraints/Linearized-Quad.jpg
       style=display:block;margin:auto;width:70%
       alt=A local linearization of a nonlinear QUAD4 element.

### Second Order Geometries (TRI6 and QUAD9 faces)

Elements defined on second order geometries are curvilinear so to simplify the 'clipping' procedure both secondary and primary face elements are subdivided into first-order face elements then subsequently linearized (illustrated below). The same clipping and triangularization routine is then applied on the linearized sub-elements to create the mortar segments. See [!cite](puso2008segment).

!media media/framework/constraints/Second-Order-Linearized.jpg
       style=display:block;margin:auto;width:70%
       alt=A second order quad element is converted to 4 first-order elements, which are then linearized and "clipped".

Quadrature points defined on mortar segments (which live on linearized elements) are mapped back to second order elements following an analogous but reverse procedure to the one illustrated above; points are mapped from linearized elements to first order sub-elements then subsequently transformed to the original second order elements.

The local plane normal for each linearized secondary subpatch is computed from the geometry of that
subpatch. Triangular subpatches use an edge cross product; quadrilateral subpatches use the cross
product of the bilinear center tangents so the normal represents the full bilinear subpatch instead
of one of its two diagonals. The geometric normal is oriented to agree with the averaged secondary
nodal normal so the primary-to-secondary orientation convention is unchanged.

For sharp 3D corners with adjacent sidesets, [!param](/Constraints/EqualValueConstraint/minimum_projection_angle)
also rejects primary and secondary subpatch pairings whose normals are too close to orthogonal
before polygon clipping. This preserves the intended face-to-face contact on each side of the corner
while avoiding lower-dimensional cross-corner coupling.

!bibtex bibliography
