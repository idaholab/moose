# Advanced Meshing Tools

!---

## ParsedCurveGenerator

The [ParsedCurveGenerator.md] object generates a 3D curve mesh composed of EDGE2 elements which connect the series of points given by $x(t)$, $y(t)$, $z(t)$. This is useful when the user wants to construct a non-standard boundary and mesh inside of it.

!row!
!col small=12 medium=6 large=8

!listing test/tests/meshgenerators/parsed_curve_generator/parsed_curve_3d.i
         block=Mesh
         link=False

!col small=12 medium=6 large=4

!media reactor/meshgenerators/xyz_curve.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## FillBetweenCurvesGenerator, FillBetweenPointVectorsGenerator, and FillBetweenSidesetsGenerator

Several mesh generators are available to the user to generate a "transition layer" between two curves using linear triangle elements (TRI3). Behind the scenes, these mesh generators use MOOSE's [FillBetweenPointVectorsTools](FillBetweenPointVectorsTools.md) capability to generate a "transition layer" between two given curves (in the form of two vectors of points).

!media framework/utils/transition_layer.png
       style=width:60%;display:block;margin-left:auto;margin-right:auto;

!---

### FillBetweenCurvesGenerator

The [FillBetweenCurvesGenerator.md] object is designed to generate a transition layer to connect two boundaries of two input meshes. The user provides two 1D meshes (curves) which should be connected to each other with transition layers.

!media reactor/meshgenerators/fill_between_curves.png
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

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
!col small=12 medium=6 large=8

!listing test/tests/meshgenerators/xy_delaunay_generator/xydelaunay_with_holes.i
         block=Mesh
         link=False

!col small=12 medium=6 large=4

!media framework/meshgenerators/poly2tri_with_holes.png
       style=width:80%;display:block;margin-left:auto;margin-right:auto;

!row-end!
