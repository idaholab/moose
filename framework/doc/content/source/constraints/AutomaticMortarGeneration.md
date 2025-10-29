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
