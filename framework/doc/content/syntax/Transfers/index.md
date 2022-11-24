# Transfers System

When running simulations that contain [MultiApps](/MultiApps/index.md)---simulations running
other sub-simulations---it is often required to move data to and from the sub-applications. Transfer
objects in MOOSE are designed for this purpose.

!alert note
Prior to understanding Transfers it is important to grasp the idea of [MultiApps](/MultiApps/index.md) first, so please
refer to the [MultiApps](/MultiApps/index.md) documentation for additional information.

## Example Transfer

Assuming that the concept of [MultiApps](/MultiApps/index.md) is understood, Transfers are best understood via an example
problem. First, consider a "parent" simulation that is solving the transient diffusion equation. This
parent simulation also includes two "sub" applications that rely on the average value of the unknown
from the parent application.

### The "parent" Simulation

[transfers-parent-multiapps] is an input file snippet showing the [MultiApps](/MultiApps/index.md) block that includes a
[TransientMultiApp](/TransientMultiApp.md), this sub-application will execute along with the parent
(at the end of each timestep) as time progresses.

!listing test/tests/transfers/multiapp_postprocessor_to_scalar/parent.i
         block=MultiApps id=transfers-parent-multiapps
         caption=The [MultiApps](/MultiApps/index.md) block of the "parent" application that contains two sub-application
                 that solves along with the parent as time progresses.

For this example, the sub-applications require that the average from the parent in the form of a
scalar AuxVariable, see the [AuxVariables] documentation for further information. Therefore the
parent will transfer the average value (computed via the
[ElementAverageValue](/ElementAverageValue.md) Postprocessor) to a scalar AuxVariable on each
sub-application. As shown in [transfers-parent-transfers], the
[MultiAppPostprocessorToAuxScalarTransfer](/MultiAppPostprocessorToAuxScalarTransfer.md) is provided
for this purpose.

!listing test/tests/transfers/multiapp_postprocessor_to_scalar/parent.i
         block=Transfers
         id=transfers-parent-transfers
         caption=The Transfers block of the "parent" application that contains a Transfer of a
                     Postprocessor to a scalar AuxVariable on sub-applications.

### The "sub" Simulations

For this simple example the sub-application must contain an appropriate AuxVariable to receiving
the Postprocessor value from the parent application.

!listing test/tests/transfers/multiapp_postprocessor_to_scalar/sub.i
         block=AuxVariables
         id=transfers-sub
         caption=The AuxVariables block of the "sub" application that contains a scalar that the
                     parent application will update.

The sub-applications do not have any "knowledge" of the parent application, and simply perform
calculations that utilize the scalar variable, regardless of how this scalar is computed. This
approach allows the sub-application input file to run in union of independent from the parent without
modification, which is useful for development and testing.

## Coordinate transformations id=coord-transform

Different applications may use different setups. For example a neutronics
simulation may be performed in Cartesian 3D space whereas a fuel performance
calculation may be performed using a 2D axisymmetric coordinate
system. Communicating information between these different configurations can be
difficult. One mechanism MOOSE provides for making this communication simpler is
the `MooseAppCoordTransform` class. Each `MooseApp` instance holds a coordinate
transformation object in its `MooseMesh` object. Users may specify
transformation information about their simulation setup on a per-application
basis in the input file `Mesh` block. The [!param](/Mesh/GeneratedMesh/coord_type)
parameter specifies the coordinate system type, e.g. XYZ, RZ, or
RSPHERICAL. [Euler angles](https://en.wikipedia.org/wiki/Euler_angles) are
available to describe extrinsic rotations. The convention MOOSE uses for
a alpha-beta-gamma Euler angle rotation is:

1. Rotation about the z-axis by [!param](/Mesh/GeneratedMesh/alpha_rotation) degrees
2. Rotation about the x-axis by [!param](/Mesh/GeneratedMesh/beta_rotation) degrees
3. Rotation about the z-axis (again) by [!param](/Mesh/GeneratedMesh/gamma_rotation) degrees


[!param](/Mesh/GeneratedMesh/length_unit) allows the user to specify
their mesh length unit. The code in `MooseAppCoordTransform`
which processes this parameter leverages the [MooseUnits](/Units.md) system. A
scaling transform will be constructed to convert a point in the mesh domain with
the prescribed mesh length unit to the reference domain with units of meters.
The last option which contributes to
transformation information held by the `MooseAppCoordTransform` class is the
[!param](/MultiApps/TransientMultiApp/positions) parameter which is described in
[MultiApps/index.md#multiapp-positions]. The value of `positions` exactly
corresponds to the translation vector set in the `MooseAppCoordTransform` object of
the sub-application. The `alpha_rotation`, `beta_rotation`, `gamma_rotation`,
and `positions` parameters essentially describe forward transformations of the
mesh domain described by the MOOSE `Mesh` block to a reference domain. Following
the ordering
[here](https://docs.microsoft.com/en-us/dotnet/desktop/winforms/advanced/why-transformation-order-is-significant?view=netframeworkdesktop-4.8),
the sequence of
transformations applied in the `MooseAppCoordTransform` class is:

1. scaling
2. rotation
3. translation
4. coordinate collapsing


The last item in the list, coordinate collapsing, is only relevant when
information has to be transferred between applications with different coordinate
systems. For transferring information from XYZ to RZ, we must collapse XYZ
coordinates into the RZ space since there is a unique mapping of XYZ coordinates
into RZ coordinates, but not vice versa, e.g. a point in RZ has infinitely many
corresponding locations in XYZ space due to rotation about the axis of
symmetry. The table below summarizes the coordinate collapses that occur when
transferring information between two different coordinate systems. The table
should be understood as follows, using the first row as an example: for a
XYZ-RZ pairing (e.g. RZ->XYZ *or* XYZ->RZ data transfers), both 'from' and 'to' points will be cast
into the RZ coordinate system for the reasoning given above: there is a unique
map from XYZ to RZ, but not vice versa. Similarly for a RZ-RSPHERICAL pairing
(e.g. RZ->RSPHERICAL *or* RSPHERICAL->RZ data transfers), both 'from' and 'to'
points will be cast into the RSPHERICAL coordinate system.

!table caption=Coordinate collapsing
| Coordinate System 1 | Coordinate System 2 | Resulting Coordinate System for Data Transfer |
| - | - | - |
| XYZ | RZ | RZ |
| XYZ | RSPHERICAL | RSPHERICAL |
| RZ | RSPHERICAL | RSPHERICAL |

Note that there are consequences for these coordinate system collapses. When
transferring data in the 1 -> 2 directions, there are (as already stated) infinitely many points
in three-dimensional Cartesian space that correspond to a single
RZ coordinate. For example, the Cartesian points (1, 0, 0) and (0, 1, 0) map to the same
RZ coordinate (1, 0) if the z-axis is the axis of symmetry on the Cartesian
mesh. So if we are performing a nearest-node transfer of data from XYZ to RZ,
where the "to" point is (1, 0), then selection of the "from" point is arbitrary
if both (1, 0, 0) and (0, 1, 0) points (or any combination of $\sqrt{x^2+y^2}=1$
points) exist. We are considering how best to handle these situations moving
forward. One option would be to average the field data from equivalent points.

Framework transfer classes that support the coordinate transformation
processes described here are:

- [MultiAppGeometricInterpolationTransfer.md]
- [MultiAppShapeEvaluationTransfer.md]
- [MultiAppNearestNodeTransfer.md]
- [MultiAppPostprocessorInterpolationTransfer.md]
- [MultiAppProjectionTransfer.md]
- [MultiAppUserObjectTransfer.md]

### Examples

Let's consider an example. The below listing shows coordinate transformation
given in the `Mesh` block of a sub-application:

!listing transfers/coord_transform/sub-app.i block=Mesh

Here, the user is stating that a -90 degree alpha rotation (e.g. a point on the
y-axis becomes a point on the x-axis) should be applied to
the sub-application's domain in order to map to the reference domain (which the user has
chosen to correspond to the main application domain). Additionally, the user
wishes for the coordinate transformation object to know that one unit of mesh
length corresponds to 20 centimeters. This information from the sub-application's `Mesh` block
combined with the translation vector described by the `positions` parameter in
the main application `MultiApp` block

!listing transfers/coord_transform/main-app.i block=MultiApps

allows MOOSE to directly map information between the disparate application
domains. The figure below shows the variable field `v`, which is a nonlinear
variable in the sub-application and an auxiliary source variable in the main
application, in both domains, indicating a successful transfer of data after
applying the transformation order outlined above (rotation, scale, translation).

!media transfers/rotated-scaled-translated.png id=transformed caption=Example of rotation, scaling, and translation transformations between multiapps


Another example leveraging the `MooseAppCoordTransform` class is a simulation in
which one application is in 3D XYZ space and another is in 2D RZ space. In this
example we wish to rotate the axis of symmetry, which is the Y-axis in the 2D RZ
simulation, in order to align with the z-axis when transferring data between the
2D RZ and 3D XYZ simulations. This simple rotation is achieved by specifying the
`beta_rotation` value below

!listing transfers/coord_transform/rz-xyz/2d-rz.i block=Mesh

We can see that the rotation transformation has been successful by examining the
same field in both applications (in this case the field is solved by the
nonlinear solver in the sub-application (variable `u`) and transferred to the
main application into the auxiliary field `v`).

!media transfers/xyz-rz.png id=multi-system caption=Example of information transformation between different coordinate system types

We mentioned how forward rotation transformations can be achieved by specifying
Euler angles. Another parameter that can be used to perform rotations is
[!param](/Mesh/GeneratedMesh/up_direction). As described in the parameter
documentation string, if `up_direction` is specified, then in the
`MooseAppCoordTransform` object we will prescribe a rotation matrix that
corresponds to a single 90 degree rotation such that a point lying on the
`up_direction` axis will now lie on the y-axis. We have chosen the y-axis to be
the canonical reference frame up-direction because it is the literal
up-direction when opening an exodus file in Paraview. Additionally it is
consistent with boundary naming for cuboidal meshes generated using
[GeneratedMesh.md] or [GeneratedMeshGenerator.md] in which the upper y-boundary
is denoted as `top`.

## Available Transfer Objects

The following is a complete list of the available Transfer objects, each links to a page with further
details.

!syntax list /Transfers objects=True actions=False subsystems=False

!syntax list /Transfers objects=False actions=False subsystems=True

!syntax list /Transfers objects=False actions=True subsystems=False
