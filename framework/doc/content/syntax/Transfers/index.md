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

## Coordinate transformations

Different applications may use different setups. For example a neutronics
simulation may be performed in Cartesian 3D space whereas a fuel performance
calculation may be performed using a 2D axisymmetric coordinate
system. Communicating information between these different configurations can be
difficult. One mechanism MOOSE provides for making this communication simpler is
the `MooseCoordTransform` class. Each `MooseApp` instance holds a coordinate
transformation object in its `FEProblemBase` object. Users may specify
transformation information about their simulation setup on a per-application
basis in the input file `Problem` block. The [!param](/Problem/FEProblem/coord_type)
parameter specifies the coordinate system type, e.g. XYZ, RZ, or
RSPHERICAL. [Euler angles](https://en.wikipedia.org/wiki/Euler_angles) are
available to describe extrinsic rotations. The convention MOOSE uses for
a alpha-beta-gamma Euler angle rotation is:

1. Rotation about the z-axis by [!param](/Problem/FEProblem/alpha_rotation) degrees
2. Rotation about the x-axis by [!param](/Problem/FEProblem/beta_rotation) degrees
3. Rotation about the z-axis (again) by [!param](/Problem/FEProblem/gamma_rotation) degrees


[!param](/Problem/FEProblem/length_units_per_meter) allows the user to specify
how many mesh length units are in a meter. The last option which contributes to
transformation information held by the `MooseCoordTransform` class is the
[!param](/MultiApps/TransientMultiApp/positions) parameter which is described in
[MultiApps/index.md#multiapp-positions]. The value of `positions` exactly
corresponds to the translation vector set in the `MooseCoordTransform` object of
the sub-application. The `alpha_rotation`, `beta_rotation`, `gamma_rotation`,
and `positions` parameters essentially describe forward transformations of the
mesh domain described by the MOOSE `Mesh` block to a reference domain. The
`length_units_per_meter` parameter effectively represents an inverse transform,
e.g. the meter is chosen as the reference scale, so a $1/l$ scaling transform
(where $l$ corresponds to `length_units_per_meter`) would be applied to the mesh
domain in order to map to the meter-based reference domain. The sequence of
transformations applied in the `MooseCoordTransform` class is:

1. rotation
2. scaling
3. translation
4. coordinate collapsing


The last item in the list, coordinate collapsing, is only relevant when
information has to be transferred between applications with different coordinate
systems. For transferring information from XYZ to RZ, we must collapse XYZ
coordinates into the RZ space since there is a unique mapping of XYZ coordinates
into RZ coordinates, but not visa versa, e.g. a point in RZ has infinitely many
corresponding locations in XYZ space due to rotation about the axis of
symmetry. The table below summarizes the coordinate collapses that occur when
transferring information between two different coordinate systems.

!table caption=Coordinate collapsing
| Coordinate System A | Coordinate System B | Resulting Coordinate System for Data Transfer |
| - | - | - |
| XYZ | RZ | RZ |
| XYZ | RSPHERICAL | RSPHERICAL |
| RZ | RSPHERICAL | RSPHERICAL |


Let's consider an example. The below listing shows coordinate transformation
given in the `Problem` block of a sub-application:

!listing transfers/coord_transform/sub-app.i block=Problem

Here, the user is stating that a -90 degree alpha rotation should be applied to
the sub-application's domain in order to map to the reference domain (which the user has
chosen to correspond to the main application domain). Additionally, the user
wishes for the coordinate transformation object to know that 5 sub-application
domain units correspond to a meter (whereas the main application is already in
units of meters). This information from the sub-application's `Problem` block
combined with the translation vector described by the `positions` parameter in
the main application `MultiApp` block

!listing transfers/coord_transform/main-app.i block=MultiApps

allows MOOSE to directly map information between the disparate application
domains. The figure below shows the variable field `v`, which is a nonlinear
variable in the sub-application and an auxiliary source variable in the main
application, in both domains, indicating a successful transfer of data after
applying the transformation order outlined above (rotation, scale, translation).

!media transfers/rotated-scaled-translated.png id=transformed caption=Example of rotation, scaling, and translation transformations between multiapps


Another example leveraging the `MooseCoordTransform` class is a simulation in
which one application is in 3D XYZ space and another is in 2D RZ space. In this
example we wish to rotate the axis of symmetry, which is the Y-axis in the 2D RZ
simulation, in order to align with the z-axis when transferring data between the
2D RZ and 3D XYZ simulations. This simple rotation is achieved by specifying the
`beta_rotation` value below

!listing transfers/coord_transform/rz-xyz/2d-rz.i block=Problem

We can see that the rotation transformation has been successful by examining the
same field in both applications (in this case the field is solved by the
nonlinear solver in the sub-application (variable `u`) and transferred to the
main application into the auxiliary field `v`).

!media transfers/xyz-rz.png id=multi-system caption=Example of information transformation between different coordinate system types

We mentioned how forward rotation transformations can be achieved by specifying
Euler angles. Another parameter that can be used to perform rotations is
[!param](/Problem/FEProblem/up_direction). As described in the parameter
documentation string, if `up_direction` is specified, then in the
`MooseCoordTransform` object we will prescribe a rotation matrix that
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
