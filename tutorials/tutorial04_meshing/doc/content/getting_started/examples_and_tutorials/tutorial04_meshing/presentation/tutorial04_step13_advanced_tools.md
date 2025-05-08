# Advanced Meshing Tools

!---

## Quadratic Element Capabilities

 Quadratic elements meshing is available for all MOOSE Reactor Module meshing tools commonly used for reactor meshing.
 
 The quadratic element options can be selected by setting `tri_element_type` as TRI6 or TRI7, and `quad_element_type` as QUAD8 or QUAD9 in the 2D mesh generators.
 
 The 2D-to-3D mesh generators such as [AdvancedExtruderGenerator.md] detect 2D quadratic elements and create 3D quadratic elements accordingly.

!media reactor/meshgenerators/pccmg_quad.png
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!---

## ParsedCurveGenerator

The [ParsedCurveGenerator.md] object generates a 3D curve mesh composed of EDGE2 elements which connect the series of points given by $x(t)$, $y(t)$, $z(t)$. This is useful when the user wants to construct a non-standard boundary and mesh inside of it.

!row!
!col! width=50%

!listing test/tests/meshgenerators/parsed_curve_generator/parsed_curve_3d.i
         block=Mesh
         link=False

!col-end!

!col! width=50%

!media reactor/meshgenerators/xyz_curve.png
       style=width:85%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!---

## FillBetweenCurvesGenerator, FillBetweenPointVectorsGenerator, and FillBetweenSidesetsGenerator

Several mesh generators are available to the user to generate a "transition layer" between two curves using linear triangle elements (TRI3). Behind the scenes, these mesh generators use MOOSE's [FillBetweenPointVectorsTools](FillBetweenPointVectorsTools.md) capability to generate a "transition layer" between two given curves (in the form of two vectors of points).

!media framework/utils/transition_layer.png
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!---

### FillBetweenCurvesGenerator

The [FillBetweenCurvesGenerator.md] object is designed to generate a transition layer to connect two boundaries of two input meshes. The user provides two 1D meshes (curves) which should be connected to each other with transition layers.

!media reactor/meshgenerators/fill_between_curves.png
       style=width:40%;display:block;margin-left:auto;margin-right:auto;

!---

### FillBetweenSidesetsGenerator

The [FillBetweenSidesetsGenerator.md] object is designed to generate a transition layer to connect two boundaries of two input meshes. The user provides two 2D input meshes with sidesets. These sidesets specify which boundaries of each mesh should be connected to the other mesh with a transitional layer.

!media framework/meshgenerators/transition_layer_stitched.png
       style=width:60%;display:block;margin-left:auto;margin-right:auto;

!---

### FillBetweenPointVectorsGenerator

This [FillBetweenPointVectorsGenerator.md] object generates a transition layer between two point vectors with different numbers of nodes. The user should provide two vectors of points as well as the number of layers of elements to create between the two vectors.

!media framework/meshgenerators/transition_layer_examples.png
       style=width:40%;display:block;margin-left:auto;margin-right:auto;

!---

## XYDelaunayGenerator

[XYDelaunayGenerator.md] creates an unstructured mesh consisting of TRI3 elements based on a given external boundary and, optionally, a series of internal hole meshes.

[XYDelaunayGenerator.md] is extremely powerful when combined with [ParsedCurveGenerator.md] and other Reactor module objects already described, as it can mesh very irregularly shaped regions.

!row!
!col! width=50%

!listing test/tests/meshgenerators/xy_delaunay_generator/xydelaunay_with_holes.i
         block=Mesh
         link=False

!col-end!

!col! width=50%

!media framework/meshgenerators/poly2tri_with_holes.png
       style=width:60%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!---

## FlexiblePatternGenerator

[FlexiblePatternGenerator.md] is designed to generate a mesh with a background region containing dispersed unit meshes that are distributed based on a series of flexible patterns. It enhances the capabilities provided in [PatternedHexMeshGenerator.md]/[PatternedCartesianMeshGenerator.md]. It can be used for irregular background shapes and/or patterning.

!row!
!col! width=50%

!listing modules/reactor/test/tests/meshgenerators/flexible_pattern_generator/mixed_pattern.i
         block=Mesh/fpg
         link=False

!col-end!

!col! width=50%

!media reactor/meshgenerators/mixed_pattern.png
       style=width:80%;display:block;margin-left:auto;margin-right:auto;


!col-end!

!row-end!

!---

## RevolveGenerator

[RevolveGenerator.md] provides an alternative tool for increasing the dimensionality of a lower dimension mesh (1D or 2D) in addition to [MeshExtruderGenerator.md]/[AdvancedExtruderGenerator.md]. Each element is converted to one or more copies of its corresponding higher dimensional element along an open or closed specific circular curve.

!row!
!col! width=50%

!listing adv_examples/mesh_revolve.i
         block=Mesh
         link=False

!col-end!

!col! width=50%

!media tutorial04_meshing/revolve_multillayer.png
       style=width:60%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!
