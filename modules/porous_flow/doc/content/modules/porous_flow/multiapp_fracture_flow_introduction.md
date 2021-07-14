# Fracture flow using a MultiApp approach: Introduction

PorousFlow can be used to simulate flow through fractured porous media in the case when the fracture network is so complicated it cannot be incorporated into the porous-media mesh.  The fundamental premise is *the fractures can be considered as lower dimensional entities within the higher-dimensional porous media*.  This has two consequences.

- Meshing is often simplified and computational speed increased.
- The fractures do not provide a barrier to flow in their normal direction.  In fact, flows (of both mass and heat) in the normal direction do not even see the fracture as they move through the porous media.  This is certainly true if the fractures have very small width (as befits a lower-dimensional entity) and flow through the fracture is substantially faster than the flow through the porous media.

A [related page](/porous_flow/nomultiapp_flow_through_fractured_media.md) describes how to simulate porous flow in fractured porous media, assuming that the fractures can be incorporated directly into the mesh as lower dimensional elements, for instance, as 2D "fracture" elements inside a 3D "matrix" mesh.  Unfortunately, realistic fracture networks have such complicated geometry that meshing them is difficult, while incorporating their mesh directly into a higher-dimensional mesh is almost impossible.
Example files are in [multiapp_fracture_flow](https://github.com/idaholab/moose/blob/master/modules/porous_flow/examples/multiapp_fracture_flow).  There is also some [non-PorousFlow documentation](https://github.com/idaholab/moose/blob/master/modules/misc/doc/fracture_flow).

In the following set of pages, it is illustrated that MOOSE's MultiApp system may be employed to solve this problem: the "fracture" mesh is governed by one App, which is seperate from the "matrix" mesh that is governed by another App.

- [Introduction](multiapp_fracture_flow_introduction.md)
- [Mathematics and physical interpretation](multiapp_fracture_flow_equations.md)
- [Transfers](multiapp_fracture_flow_transfers.md)
- [MultiApp primer](multiapp_fracture_flow_primer.md): the diffusion equation with no fractures, and quantifying the errors introduced by the MultiApp approach
- [Diffusion in mixed dimensions](multiapp_fracture_flow_diffusion.md)
- [Porous flow in a single matrix system](multiapp_fracture_flow_PorousFlow_2D.md)
- [Porous flow in a small fracture network](multiapp_fracture_flow_PorousFlow_3D.md)

!alert note
[Kuzmin-Turek](/porous_flow/kt.md) stabilization cannot typically be used for the fracture flow.  This is because multiple fracture elements will meet along a single finite-element edge when fracture planes intersect, while for performance reasons, libMesh assumes only one or two elements share an edge.  This prevents the KT approach from quantifying flows in all neighboring elements, which prevents the scheme from working.

!alert note
The MultiApp approach typically breaks the unconditional stability of MOOSE's implicit time-stepping scheme.  This is because usually the fracture physics is "frozen" while the matrix physics is evolving, and vice-versa.  This can easily lead to oscillations if the time-step is too big.
