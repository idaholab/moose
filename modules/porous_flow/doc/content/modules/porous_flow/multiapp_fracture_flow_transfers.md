# Fracture flow using a MultiApp approach: Transfers

PorousFlow can be used to simulate flow through fractured porous media in the case when the fracture network is so complicated it cannot be incorporated into the porous-media mesh.  The fundamental premise is *the fractures can be considered as lower dimensional entities within the higher-dimensional porous media*.  This page is part of a set that describes a MOOSE MultiApp approach to simulating such models:

- [Introduction](multiapp_fracture_flow_introduction.md)
- [Mathematics and physical interpretation](multiapp_fracture_flow_equations.md)
- [Transfers](multiapp_fracture_flow_transfers.md)
- [MultiApp primer](multiapp_fracture_flow_primer.md): the diffusion equation with no fractures, and quantifying the errors introduced by the MultiApp approach
- [Diffusion in mixed dimensions](multiapp_fracture_flow_diffusion.md)
- [Porous flow in a single matrix system](multiapp_fracture_flow_PorousFlow_2D.md)
- [Porous flow in a small fracture network](multiapp_fracture_flow_PorousFlow_3D.md)

A number of different Transfers are used in the MultiApp fracture flow simulations.  This page explains the functionality of each type.  Remember that the [TransientMultiApp](TransientMultiApp.md) has the capability to `interpolate_transfers` while sub-cycling.  This feature is not used in any of the above pages, but could be employed in more sophisticated models to reduce oscillatory behaviour.

## MultiAppReporterTransfer

A [NodalValueSampler](NodalValueSampler.md) VectorPostprocessor defined on the fracture app records, at each fracture node, the heat transfer from the fracture to the matrix.  A [MultiAppReporterTransfer](MultiAppReporterTransfer.md) transfers this information to a `VectorPostprocessor` on the matrix app.


## MultiAppShapeEvaluationTransfer

A [MultiAppShapeEvaluationTransfer](MultiAppShapeEvaluationTransfer.md) is used to transfer the matrix temperature to the fracture mesh.  The matrix shape functions are used to interpolate the matrix temperature to the fracture-node positions, as shown in [meshfunction_matrix_to_fracture].  Because it does no extrapolation, this transfer is only fit for use when the matrix domain contains the fracture domain (no fracture elements are "poking out" of the matrix mesh, but are all "inside" the matrix mesh).  If extrapolation is required, a [MultiAppGeometricInterpolationTransfer](MultiAppGeometricInterpolationTransfer.md) must be used.

!media porous_flow/examples/multiapp_flow/meshfunction_matrix_to_fracture.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=meshfunction_matrix_to_fracture
	caption=Temperature (indicated by colors) is transferred from the matrix (6 square elements) to the fracture (zig-zag line) using a MultiAppShapeEvaluationTransfer.  Fracture nodes lying outside the matrix domain do not get prescribed a value by the Transfer, so remain at their initial values (zero in this case).

## MultiAppGeometricInterpolationTransfer

A [MultiAppGeometricInterpolationTransfer](MultiAppGeometricInterpolationTransfer.md) may be used to transfer the matrix temperature to the fracture mesh, as shown in [interpolation_matrix_to_fracture].  Each node on the fracture mesh looks for the nearest `num_points` (3 by default) nodes on the matrix mesh, and receives a temperature value interpolated from those three temperatures.  As mentioned above, an alternative is to use a [MultiAppShapeEvaluationTransfer](MultiAppShapeEvaluationTransfer.md) which uses the matrix shape-function information instead, but that requires the fracture domain to be included within the matrix domain, since no extrapolation is performed.

!media porous_flow/examples/multiapp_flow/interpolation_matrix_to_fracture.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=interpolation_matrix_to_fracture
	caption=Temperature (indicated by colors) is transferred from the matrix to the fracture using a MultiAppShapeEvaluationTransfer.  Fracture nodes lying outside the matrix domain get prescribed a value by the Transfer, and the fracture nodes lying inside the matrix domain depend on the 3 (by default) closest matrix temperature values.

## MultiAppNearestNodeTransfer

A [MultiAppNearestNodeTransfer](MultiAppNearestNodeTransfer.md) transfers information regarding fracture-normal directions, element lengths and thermal conductivities.  All of these are elemental quantities, viz `CONSTANT MONOMIAL` `AuxVariables`.

### Fracture-normal directions

Each matrix element looks for its closest fracture element and retrieves the normal vector from that fracture element, as in [nearestnode_fracture_to_matrix].  Matrix element distant from all fractures retrieve this information too, but that never impacts the simulation results.  There may be performance implications for very large meshes as quite a number of searches must be performed to build the required information.

!media porous_flow/examples/multiapp_flow/nearestnode_fracture_to_matrix.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=nearestnode_fracture_to_matrix
	caption=A CONSTANT MONOMIAL field (indicated by colors) is transferred from the fracture (zig-zag line) to the matrix (6 square elements) using a MultiAppNearestNodeTransfer.  For clarity, this example is transferring an arbitrary variable called `fracture_var`, not fracture normal directions.  The small white lines indicate which fracture element is contributing to each matrix element (the white lines are not normal directions).


Note that a [MultiAppGeometricInterpolationTransfer](MultiAppGeometricInterpolationTransfer.md) could be used instead, but if the matrix elements are small compared with the fracture elements, then the information received by each matrix element would be influenced by distant fracture elements.  On the other hand, if matrix elements are large compared with fracture elements, then a [MultiAppGeometricInterpolationTransfer](MultiAppGeometricInterpolationTransfer.md) could provide a better estimate of average fracture-normal direction within each matrix element.

### Element lengths and thermal conductivities

Each fracture element looks for its closest matrix element and retrieves the element normal-length and normal thermal conductivity, as in [nearestnode_matrix_to_fracture].

!media porous_flow/examples/multiapp_flow/nearestnode_matrix_to_fracture.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=nearestnode_matrix_to_fracture
	caption=A CONSTANT MONOMIAL field (indicated by colors) is transferred from the matrix to the fracture using a MultiAppNearestNodeTransfer.  Even fracture elements lying outside the matrix domain get prescribed a value by the Transfer.

Note that a [MultiAppGeometricInterpolationTransfer](MultiAppGeometricInterpolationTransfer.md) could provide a better estimate of element length and conductivity, if the matrix elements are small compared with the fracture elements.  On the other hand, if matrix elements are large compared with fracture elements, then using a [MultiAppGeometricInterpolationTransfer](MultiAppGeometricInterpolationTransfer.md) would mean the information received by each fracture element would be influenced by distant matrix elements.

## MultiAppCopyTransfer

This copies values between identical meshes, so is not used for the mixed-dimensional fracture flow examples.
