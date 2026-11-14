# ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial

!syntax description /FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial

## Description

`ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial` defines the continuum surface force
(CSF) of a diffuse interface. From the volume fraction $\alpha$ of
[!param](/FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial/volume_fraction_functor)
and the surface tension coefficient $\sigma$ of
[!param](/FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial/surface_tension_coefficient),
it computes the interface curvature and the surface tension force density

\begin{equation}
\kappa = -\nabla \cdot \hat{\mathbf{n}},
\qquad
\hat{\mathbf{n}} = \frac{\nabla \alpha}{|\nabla \alpha| + \epsilon},
\qquad
\mathbf{f}_{st} = \sigma \kappa \nabla \alpha,
\end{equation}

where $\epsilon$ is
[!param](/FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial/normal_regularization),
which must be positive.

The cell gradient of $\alpha$ is a least-squares gradient over the neighboring cells that carry
the volume fraction. The curvature of a cell is the divergence of the unit normal, assembled as
the flux of the face normals through the cell's faces; on an internal face the normal is formed
from the average of the two cell gradients. The force density of a cell is
$\sigma \kappa \nabla \alpha$ with the cell's own curvature and gradient, and it is zero wherever
$\sigma$ is zero.

Both quantities are cell quantities. On an internal face they are the average of the two cell
values; on a boundary face they are the value of the cell on the side that carries the volume
fraction. They have no nodal values. They are published as the functors named
[!param](/FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial/interface_curvature_name)
and
[!param](/FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial/surface_tension_force_density_name),
and are evaluated on demand from the current volume fraction.

[WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics](WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.md)
creates this material when its `surface_tension_coefficient` is set, which also requires its
`volume_fraction_functor`. It passes the force density to the
[ConservativeDiffuseInterfaceRhieChowMassFlux](ConservativeDiffuseInterfaceRhieChowMassFlux.md),
which adds it to the momentum predictor and carries it into the pressure equation as a face flux.

## Example Input Syntax

In this example, a circular liquid droplet with surface tension coefficient `sigma` rests in gas
without gravity. The flow physics `flow` creates this material from the volume fraction `alpha`,
and the pressure jump across the interface matches the Young-Laplace value $\sigma / R$.

!listing modules/navier_stokes/test/tests/finite_volume/two_phase/diffuse_interface/static_droplet_laplace_pressure.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated

!syntax parameters /FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial

!syntax inputs /FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial

!syntax children /FunctorMaterials/ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial
