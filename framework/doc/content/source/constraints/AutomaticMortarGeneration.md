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

## Nodal normal coordinate sensitivities

`AutomaticMortarGeneration` constructs one unit normal at each secondary mortar node by normalizing
the sum of the oriented, nodal-quadrature area vectors from all incident secondary faces. For
the supported quasistatic local-basis `mortar` and non-augmented `mortar_penalty`
mechanical-contact formulations, Jacobian-bearing evaluations also construct a sparse
coordinate-sensitivity stencil

!equation
\delta \boldsymbol{n}_A =
\frac{\boldsymbol{I} - \boldsymbol{n}_A \otimes \boldsymbol{n}_A}
     {\lVert \boldsymbol{a}_A \rVert}
\delta \boldsymbol{a}_A,

where \(\boldsymbol{a}_A\) is the unnormalized sum of the oriented edge or face area vectors at
node \(A\). In two dimensions, differentiating an area vector is equivalent to differentiating and
rotating the edge tangent. In three dimensions, each face contribution uses

!equation
\delta(\boldsymbol{x}_{,\xi} \times \boldsymbol{x}_{,\eta}) =
\delta\boldsymbol{x}_{,\xi} \times \boldsymbol{x}_{,\eta} +
\boldsymbol{x}_{,\xi} \times \delta\boldsymbol{x}_{,\eta}.

The stencil includes coordinates from every secondary face incident to the normal node. It is built
on demand for Jacobian-bearing evaluations. Residual-only evaluations continue to use the same stored
nodal-normal values without populating the AD geometry cache. The degenerate-edge check scales the edge tangent
magnitude by the element characteristic length, while the degenerate-face check scales the face
area-vector magnitude by the squared characteristic length. The cancellation check compares the
summed area-vector magnitude with the sum of the incident area-vector magnitudes. These relative
checks are independent of the mesh coordinate units.
Matching nodal-quadrature locations to secondary nodes uses the local element size when setting its
geometric tolerance.

The linearization treats the set of incident faces and each face-orientation sign as fixed during a
Newton solve. Changes to interface connectivity, face-star membership, or orientation are discrete
geometry updates and are not part of this coordinate-sensitivity stencil.

Coordinate sensitivities are implemented for `EDGE2`, `EDGE3`, `TRI3`, `TRI6`, `TRI7`, `QUAD4`,
`QUAD8`, and `QUAD9` secondary mortar elements. A supported Jacobian evaluation on any other
secondary element type reports an error rather than silently omitting derivatives.

## 2D

Generation of the 2D mortar segment mesh is outlined in [!cite](osti_1468630). In short, a nodal-normal projection is used to map points from the primary interface to the secondary interface; secondary interface elements are then split by the projected nodes to form mortar segment mesh elements.

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

!bibtex bibliography
