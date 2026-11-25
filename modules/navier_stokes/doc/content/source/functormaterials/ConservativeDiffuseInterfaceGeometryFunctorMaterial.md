# ConservativeDiffuseInterfaceGeometryFunctorMaterial

!syntax description /FunctorMaterials/ConservativeDiffuseInterfaceGeometryFunctorMaterial

## Description

`ConservativeDiffuseInterfaceGeometryFunctorMaterial` defines the unit normal of a diffuse
interface from the gradient of its volume fraction $\alpha$, given by
[!param](/FunctorMaterials/ConservativeDiffuseInterfaceGeometryFunctorMaterial/volume_fraction_functor):

\begin{equation}
\hat{\mathbf{n}} = \frac{\nabla \alpha}{|\nabla \alpha| + \delta_n},
\end{equation}

where $\delta_n$ is
[!param](/FunctorMaterials/ConservativeDiffuseInterfaceGeometryFunctorMaterial/delta_n), a small
positive regularization that keeps the normal finite where the gradient vanishes, away from the
interface. It must be positive.

On an internal face whose two cells both carry the volume fraction, the face gradient is the
average of the two cell gradients. Elsewhere, and at cell centers, the gradient of the volume
fraction functor itself is used. The normal is published as a vector functor named
[!param](/FunctorMaterials/ConservativeDiffuseInterfaceGeometryFunctorMaterial/interface_unit_normal_name).
It is evaluated on demand, so it always follows the current volume fraction.

The volume-fraction transport of
[WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics](WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics.md)
reads this face normal as its `interface_normal_functor` to orient the interface compression
term of the [ConservativeDiffuseInterfaceVOFMULESCorrector](ConservativeDiffuseInterfaceVOFMULESCorrector.md).

[WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics](WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.md)
creates this material when its `volume_fraction_functor` is set and `create_geometry_functors` is
true (the default). The normal is then named after the physics, for example
`flow_interface_unit_normal_face` for a physics block named `flow`, and $\delta_n$ is the
physics' `geometry_delta_n`.

## Example Input Syntax

In this example, the flow physics `flow` creates this material from the volume fraction `alpha`,
and the volume-fraction physics `vof` uses the resulting `flow_interface_unit_normal_face` to
compress the interface of a static droplet.

!listing modules/navier_stokes/test/tests/finite_volume/two_phase/diffuse_interface/static_droplet_laplace_pressure.i block=Physics/NavierStokes

!syntax parameters /FunctorMaterials/ConservativeDiffuseInterfaceGeometryFunctorMaterial

!syntax inputs /FunctorMaterials/ConservativeDiffuseInterfaceGeometryFunctorMaterial

!syntax children /FunctorMaterials/ConservativeDiffuseInterfaceGeometryFunctorMaterial
