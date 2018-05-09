# GeneralizedPlaneStrainUserObject

!syntax description /UserObjects/GeneralizedPlaneStrainUserObject

The `GeneralizedPlaneStrainUserObject` calculates the values of residual and diagonal jacobian components for each given scalar out-of-plane strain variable. This object is usually set up by the [GeneralizedPlaneStrainAction](/GeneralizedPlaneStrainAction.md).

For a given scalar out-of-plane strain variable, the equilibrium condition condition in the out-of-plane direction (e.g. $z$-direction) is given as
\begin{equation}
	\int_{A}{\sigma_{zz}dA} = \bar{N}_{zz}
\end{equation}

Thus, the residual corresponding to the scalar out-of-plane strain variable is

\begin{equation}
	R = \int_{A}{\sigma_{zz}dA} - \bar{N}_{zz}
\end{equation}

and the corresponding diagonal jacobian is

\begin{equation}
	K_{zz} = \frac{\partial R_{zz}}{\partial \epsilon_{zz}} = \int_{A}{\frac{\partial \sigma_{zz}}{\partial \epsilon_{zz}}dA} = \int_{A}{C_{2222}dA}
\end{equation}

A detailed description of generalized plane strain model can be found in the [formulation](tensor_mechanics/generalized_plane_strain.md) page.

!syntax parameters /UserObjects/GeneralizedPlaneStrainUserObject

!syntax inputs /UserObjects/GeneralizedPlaneStrainUserObject

!syntax children /UserObjects/GeneralizedPlaneStrainUserObject
