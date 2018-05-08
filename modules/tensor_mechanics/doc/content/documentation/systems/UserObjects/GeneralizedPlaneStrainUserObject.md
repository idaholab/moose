# GeneralizedPlaneStrainUserObject

!syntax description /UserObjects/GeneralizedPlaneStrainUserObject

UserObject `GeneralizedPlaneStrainUserObject` calculates the values of residual and diagonal jacobian components for each given scalar out-of-plane strain variable.

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
	J = \frac{\partial R_{zz}}{\partial \epsilon_{zz}} = \int_{A}{\frac{\partial \sigma_{zz}}{\partial \epsilon_{zz}}dA} = \int_{A}{C_{2222}dA}
\end{equation}

A detailed description of generalized plane strain formulation can be found in [here](tensor_mechanics/generalized_plane_strain.md).

!syntax parameters /UserObjects/GeneralizedPlaneStrainUserObject

!syntax inputs /UserObjects/GeneralizedPlaneStrainUserObject

!syntax children /UserObjects/GeneralizedPlaneStrainUserObject
