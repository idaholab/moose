# Generalized Plane Strain Scalar Kernel

!syntax description /ScalarKernels/GeneralizedPlaneStrain

The `GeneralizedPlaneStrain` ScalarKernel assembles components of the residual and diagonal jacobian corresponding to a given scalar variable. The values of the residual and diagonal jacobian components are fetched from the [GeneralizedPlaneStrainUserObject](/GeneralizedPlaneStrainUserObject.md). This object is usually set up by the [GeneralizedPlaneStrainAction](TensorMechanics/GeneralizedPlaneStrain/index.md).

The equilibrium condition when the out-of-plane direction is the $x$-direction is given as
\begin{equation}
	\int_{A}{\sigma_{xx}dA} = \bar{N}_{xx}
\end{equation}

The equilibrium condition when the out-of-plane direction is the $y$-direction is given as
\begin{equation}
	\int_{A}{\sigma_{yy}dA} = \bar{N}_{yy}
\end{equation}

The equilibrium condition when the out-of-plane direction is the $z$-direction is given as
\begin{equation}
	\int_{A}{\sigma_{zz}dA} = \bar{N}_{zz}
\end{equation}


A detailed description of generalized plane strain formulation can be found in [here](tensor_mechanics/generalized_plane_strain.md).

!syntax parameters /ScalarKernels/GeneralizedPlaneStrain

!syntax inputs /ScalarKernels/GeneralizedPlaneStrain

!syntax children /ScalarKernels/GeneralizedPlaneStrain

!bibtex bibliography
