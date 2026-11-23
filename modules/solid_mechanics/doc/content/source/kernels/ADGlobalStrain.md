# ADGlobalStrain

!syntax description /Kernels/ADGlobalStrain

## Description

### Residual

The `ADGlobalStrain` Kernel evaluates

\begin{equation}
R_i = \int{(\sigma_{ij} - \sigma_{ij}^{applied})dV}
\end{equation}

for periodic direction $i$.

### Scalar Variable Assignment

The scalar varibles fed the Kernel must have `order` set by the number of periodic components required for the mesh size.

$\epsilon_{ij}$ is active if $p_i \vee p_j$; where $i \not = j$

!table id=1 caption=Scalar Variable Assignment Guide
| Mesh Dimension | Periodicity | Diagonal Index | Off-Diagonal Index | Diagonal Variable Order | Off-Diagonal Variable Order |
| --- | --- | --- | --- | --- | --- |
| 1 | x | xx | N/A | 1 | N/A |
| 2 | x | xx | xy | 1 | 1 |
| 2 | x,y | xx,yy | xy | 2 | 1 |
| 3 | x | xx | xy,xz | 1 | 2 |
| 3 | x,y | xx,yy | xy,xz,yz | 2 | 3 |
| 3 | x,y,z | xx,yy,zz | xy,xz,yz | 3 | 3 |

For N/A, ommit an off-diagonal Kernel and Scalar Variable.

### Output Differences

The AD and non-AD formulations of Global Strain will produce the same stress/strain state, but will differ by a rigid body rotation. This occurs as the non-AD formulation outputs the engineering shear strain while the AD formulation outputs the tensor shear strain. The output differences only occur when not all directions are periodic and a shear load is present in a non-periodic direction.

The strains are related via:

Let $\epsilon_{ij}$ be the tensor shear strain component $ij$.

Let $\gamma_{ij}$ be the engineering shear strain between components $i$ $\perp$ $j$.

\begin{equation}
\epsilon_{ij} = \frac{1}{2}(\frac{\partial{u}}{\partial{x_i}} + \frac{\partial{u}}{\partial{x_j}})
\end{equation}

and

\begin{equation}
\gamma = 2\epsilon_{ij}
\end{equation}

## Nonlinear Variable
The Kernel system requires all kernels to act on a variable within the system. For the ADGlobalStrain kernel, the `variable` parameter is there to satisfy this requirement and as such non residual is calculated on the chosen `variable` parameter. In the example files, the chosen variable is u_x but the kernel does not change the residual calculation for u_x as it is explicity turned off in the source code.


## AD Global Strain Objects

Used with the automatic differentiation version of Global Strain.

[ADComputeGlobalStrain](/ADComputeGlobalStrain.md)

[ADGlobalDisplacementAux](/ADGlobalDisplacementAux.md)

[GlobalStrainPeriodicDirUserObject](/GlobalStrainPeriodicDirUserObject.md)

!alert note
Global strain only works for small strain formulations. This applies to the AD and non-AD versions.

!syntax parameters /Kernels/ADGlobalStrain

!syntax inputs /Kernels/ADGlobalStrain

!syntax children /Kernels/ADGlobalStrain
