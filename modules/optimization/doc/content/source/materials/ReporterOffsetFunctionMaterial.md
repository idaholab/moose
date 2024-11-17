# ReporterOffsetFunctionMaterial

!syntax description /Materials/ReporterOffsetFunctionMaterial

This creates a material that is the sum of a function shifted by a set of points contained in a reporter.  The reporter can contain points by specifying [!param](/Materials/ReporterOffsetFunctionMaterial/point_name) or by three seperate reporters containing the (x,y,z) coordinates of the points using [!param](/Materials/ReporterOffsetFunctionMaterial/x_coord_name), [!param](/Materials/ReporterOffsetFunctionMaterial/y_coord_name), and [!param](/Materials/ReporterOffsetFunctionMaterial/z_coord_name). This can be useful for creating a field containing multiple sources, as shown in [offset] and described in the equation below.


!equation
\tilde f(\mathbf{x}) = \sum_{i=1}^{n} \left(f(\mathbf{x} - \mathbf{p}_{offset}) \right)

!media large_media/framework/materials/offset_func.png
       id=offset
       caption=Field with multiple offsets of (left) gaussian function and (right) constant circle function.

## Example Input File Syntax

In this example, `ReporterOffsetFunctionMaterial` is used to define the two materials shown in [offset]. The value at a point is the sum of all the shifted functions.  In this example, the gaussian and constant circle functions are defined at (x,y,z)=(0,0,0), shown by the source in the lower left corner of each domain.  This function is then shifted according to the offset vectors given by the Reporter.

!listing test/tests/optimizationreporter/reporter_offset/reporter_offset_mat.i block=Materials/multiple_sources

!syntax parameters /Materials/ReporterOffsetFunctionMaterial

!syntax inputs /Materials/ReporterOffsetFunctionMaterial

!syntax children /Materials/ReporterOffsetFunctionMaterial
