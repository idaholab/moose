# ControlDrumMaterial

!syntax description /Materials/ControlDrumMaterial

## Description

`ControlDrumMaterial` supports evaluations of a material property based on the current rotation position(s) of the control drum(s).
The material operates on a mesh subdomain representing control drums(s).
There could be multiple control drums in this material indicated by their rotation centers in [!param](/Materials/ControlDrumMaterial/rotation_centers).
The code can automatically recogonize which drum that an mesh element belongs to based on the distances between the element and the rotation centers.
The drum that an mesh element belongs to has the minimum distance between its rotation center and the average of element vertices.
The control drums may have multiple *segements* rotation-wise in [!param](/Materials/ControlDrumMaterial/segment_angles), although all drums must have the same number of segments.
Values in [!param](/Materials/ControlDrumMaterial/segment_angles) must sum to 360 degree to cover the entire rotation range.
The rotation positions of all drums, or more precisely the rotation angle of the starting edge of the first segment, are specified by functors in [!param](/Materials/ControlDrumMaterial/rotation_angle_functors) and [!param](/Materials/ControlDrumMaterial/rotation_angle_offsets).
The size of [!param](/Materials/ControlDrumMaterial/rotation_angle_functors) can be equal to the number of rotating components or one when all components share the same function.
However, the size of [!param](/Materials/ControlDrumMaterial/rotation_angle_offsets) must be equal to the number of rotating components.

!alert note
The drum rotation position is typically specified with a function or a postprocessor as a functor.
A function that has spatial dependency should not be used.

!alert tip
Users can let several components share the same rotation angle function with or without different angle offsets to lock the rotation of these components together.

The code also allows users to choose the rotation axis with [!param](/Materials/ControlDrumMaterial/rotation_axis).
Only z or -z are allowed for a two-dimensional mesh.
The rotation follows the right-thumb rule, i.e. z represents counter-clock-wise rotation while -z is for clock-wise rotation.

Multiple material properties, whose name is specified with [!param](/Materials/ControlDrumMaterial/drum_material_properties), are evaluated from coupled material properties of all segments whose names are listed in a two-dimensional parameter [!param](/Materials/ControlDrumMaterial/segment_material_properties).
The leading size of [!param](/Materials/ControlDrumMaterial/segment_material_properties) must be equal to the number of property names in [!param](/Materials/ControlDrumMaterial/drum_material_properties).
When the code visits a quadrature point in an element, it first determines which drum the element belongs to.
Then it evaluates the angle between the line connecting the quadrature point and the rotation center and the base coordinate axis (x when [!param](/Materials/ControlDrumMaterial/rotation_axis) is equal to $\pm z$ for example).
Then, it determines which segment this quadrature point belongs to based on the angle, the current rotation position by [!param](/Materials/ControlDrumMaterial/rotation_angle_functors) and [!param](/Materials/ControlDrumMaterial/rotation_angle_offsets).
Finally, it assigns the drum material properties with the corresponding segment material properties in [!param](/Materials/ControlDrumMaterial/segment_material_properties).

!alert note
The material requires that material properties of all segments are available on all quadrature points in its mesh subdomain although only one of them is chosen for the drum material property at a particular quadrature point.
There could be situations in which quadrature points within a single element belongs to different segments.
Material property values are different on the quadrature points.
The integration of the material property on the element divided by element volume is an approximation of the volume homogenized property.
These typically do not introduce significant discretization errors for thermal conduction, but does require some special treatment for neutroncs often known as decusping.

!alert note
The material only evaluates the material property according to the rotation position.
It does not consider the physics due to the component rotation during transient.
A convection term with component rotation speed needs to be introduced for completeness, but this physics can often be neglected in modeling efforts.

!syntax parameters /Materials/ControlDrumMaterial

!syntax inputs /Materials/ControlDrumMaterial

!syntax children /Materials/ControlDrumMaterial
