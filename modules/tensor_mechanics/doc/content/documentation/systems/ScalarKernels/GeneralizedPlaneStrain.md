# GeneralizedPlaneStrain

!syntax description /ScalarKernels/GeneralizedPlaneStrain

The `GeneralizedPlaneStrain` ScalarKernel assembles components of the residual and diagonal jacobian corresponding to a given scalar variable. The values of the residual and diagonal jacobian components are fetched from the [GeneralizedPlaneStrainUserObject](/GeneralizedPlaneStrainUserObject.md). This object is usually set up by the [GeneralizedPlaneStrainAction](/GeneralizedPlaneStrainAction.md).

The equilibrium condition condition in the out-of-plane direction is given as
\begin{equation}
	\int_{A}{\sigma_{zz}dA} = \bar{N}_{zz}
\end{equation}

A detailed description of generalized plane strain formulation can be found in [here](tensor_mechanics/generalized_plane_strain.md).

!syntax parameters /ScalarKernels/GeneralizedPlaneStrain

!syntax inputs /ScalarKernels/GeneralizedPlaneStrain

!syntax children /ScalarKernels/GeneralizedPlaneStrain

!bibtex bibliography
