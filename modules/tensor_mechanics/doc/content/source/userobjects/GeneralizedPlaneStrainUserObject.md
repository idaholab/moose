# Generalized Plane Strain User Object

!syntax description /UserObjects/GeneralizedPlaneStrainUserObject

The `GeneralizedPlaneStrainUserObject` calculates the values of residual and diagonal jacobian components for each given scalar out-of-plane strain variable. This object is usually set up by the [GeneralizedPlaneStrainAction](TensorMechanics/GeneralizedPlaneStrain/index.md).

For a given scalar out-of-plane strain variable, the equilibrium condition in the out-of-plane direction (e.g. $z$-direction) is given as
\begin{equation}
	\int_{A}{\sigma_{zz}dA} = \bar{N}_{zz}
\end{equation}
where $\bar{N}_{zz}$ is an externally applied force.  Thus, the residual corresponding to the scalar out-of-plane strain variable is
\begin{equation}
	R = \int_{A}{\sigma_{zz}dA} - \bar{N}_{zz}
\end{equation}
and the corresponding diagonal jacobian is
\begin{equation}
	K_{zz} = \frac{\partial R_{zz}}{\partial \epsilon_{zz}} = \int_{A}{\frac{\partial \sigma_{zz}}{\partial \epsilon_{zz}}dA} = \int_{A}{C_{2222}dA}
\end{equation}

The externally applied force, $\bar{N}_{zz}$, can be imposed as the integral of a pressure applied over the area. This pressure can be imposed using either a function (using the `out_of_plane_pressure_function` parameter), or a material property (using the `out_of_plane_pressure_material` parameter).

The reference residual value used by [GeneralizedPlaneStrainReferenceResidual](/GeneralizedPlaneStrainReferenceResidual.md) is computed as
\begin{equation}
	R_{ref} = \int_{A}{|\sigma_{zz}|dA}
\end{equation}

This formulation is also used when the out-of-plane direction is the $x$-direction or $y$-direction with the subscripts in the preceding equations being changed from $zz$ to $xx$ or $yy$, respectively.

A detailed description of generalized plane strain model can be found in the [formulation](tensor_mechanics/generalized_plane_strain.md) page.

!syntax parameters /UserObjects/GeneralizedPlaneStrainUserObject

!syntax inputs /UserObjects/GeneralizedPlaneStrainUserObject

!syntax children /UserObjects/GeneralizedPlaneStrainUserObject
