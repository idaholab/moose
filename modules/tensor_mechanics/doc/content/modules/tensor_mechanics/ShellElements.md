# Shell elements

A shell element is used to model the response of structural elements that are much thinner along one direction (out-of-plane direction) compared to the two in-plane directions. A 4-node shell element is implemented in MOOSE based on [!cite](dvorkin1984continuum) that can model structural response of both thin and thick plates.

!media media/tensor_mechanics/shell_element.png
      style=width:50%;margin-left:2%;float:right
      id=fig_shell
      caption=Shell element with 4 nodes and 3 translational and 2 rotational degrees of freedom at each node.

Each of the four nodes have 5 degrees of freedom - 3 displacements ($u_1^k$,$u_2^k$, $u_3^k$) and 2 rotations ($\alpha_k$ and $\beta_k$). The thickness ($a_k$) is assumed to be constant across the shell element and the initial normal ($^oV_n^k$) at all nodes are same as the normal to the shell element, which is automatically computed using the coordinates of the nodes.

The basic equation of equilibrium for a quasi-static shell is the same as that of a continuum brick element:

\begin{equation}
\nabla \cdot \sigma = F_{ext}
\end{equation}

The quasi-static shell element implementation is MOOSE is spread across four different objects: strain computation ([ADComputeIncrementalShellStrain](/ADComputeIncrementalShellStrain.md) & [ADComputeFiniteShellStrain](/ADComputeFiniteShellStrain.md)), elasticity tensor computation ([ADComputeIsotropicElasticityTensorShell](/ADComputeIsotropicElasticityTensorShell.md)), stress computation ([ADComputeShellStress](/ADComputeShellStress.md)) and stress divergence kernel ([ADStressDivergenceShell](/ADStressDivergenceShell.md)).


## Strain Calculation

The first step to calculating stresses is to calculate the strains in the shell element. All the computations are performed in the natural coordinate system of the shell ($r_1$, $r_2$ and $r_3$). The geometry of the element at any time t is defined as:

\begin{equation}
^t x_i =  \sum_{k=1}^4 h_k ^t {x_i}^k + \frac{r_3}{2} \sum_{k=1}^4 a_k h_k ^tV_{ni}^k
\end{equation}

Therefore, the incremental displacements of any point within the shell element with natural coordinate $r_i$ at time $t$ in the global cartesian coordinate system ($^ox_1$, $^ox_2$ and $^ox_3$) are:

\begin{equation}
u_i = \sum_{k=1}^4 h_k u_i^k + \frac{r_3}{2} \sum_{k=1}^4 a_k h_k (-^tV_{2i}^k \alpha_k + ^tV_{1i}^k \beta_k)
\end{equation}

If $^t \mathbf{g_i} = \partial ^t \mathbf{x}/\partial r_i$ are the covariant base vectors, then the Green-Lagrange strain components can be written as:

\begin{equation}
\tilde{\epsilon}_{ij} = \frac{1}{2}(^t \mathbf{g_i} \cdot ^t \mathbf{g_j} - ^0 \mathbf{g_i} \cdot ^0 \mathbf{g_j})
\end{equation}

The expressions for the small and large strains in the 11, 22, 12, 13 and 23 directions are provided in equations 21 to 24 of [!cite](dvorkin1984continuum). The expressions for the shear strains in the 13 and 23 directions also include a correction for shear locking. In MOOSE the small strain increments are computed in
[ADComputeIncrementalShellStrain](/ADComputeIncrementalShellStrain.md) and the small + large strain increments are computed in [ADComputeFiniteShellStrain](/ADComputeFiniteShellStrain.md)). Note that strain in the 33 direction is not computed.

Apart from the strain increment, two other matrices (B and BNL) are computed by the strain class. These two matrices connect the nodal displacements/rotations to the small and large strains, respectively, i.e., $e=B u$ and $\eta = B_{NL} u$. These two matrices are then used to compute the residuals in [ADStressDivergenceShell](/ADStressDivergenceShell.md).

## Elasticity tensor

The shell element assumes a plane stress condition, i.e., the normal stress in the 33 direction should be zero. To enable the plane stress condition, an edited constitutive tensor is created in the local cartesian coordinate system ($\hat{C}_{mnop}$). This local constitutive tensor is then transformed into contravariant constitutive tensor using:

\begin{equation}
\tilde{C}_{ijkl} = (g^i \cdot \hat{e}_m)(g^j \cdot \hat{e}_n)(g^k \cdot \hat{e}_o)(g^l \cdot \hat{e}_p) \hat{C}_{mnop}
\end{equation}

where $g^i$ are the contravariant base vectors and $\hat{e}_i$ are the local cartesian coordinate system computed using the covariant base vectors, i.e.,

\begin{equation}
\hat{e}_3 = \frac{g_3}{|g_3|}; \hat{e}_1 = \frac{g_2 \times \hat{e}^3}{|g_2 \times \hat{e}^3|}; \hat{e}_2 = \hat{e}_3 \times \hat{e}_1
\end{equation}

The elasticity tensor at each quadrature point (qp) is computed in [ADComputeIsotropicElasticityTensorShell](/ADComputeIsotropicElasticityTensorShell.md).

## Stress calculation

The stress tensor at each qp is computed as:

\begin{equation}
\tilde{\tau}^{ij} = \tilde{C}^{ijkl} \tilde{\epsilon}_{kl}
\end{equation}

The total stress at each qp is computed in [ADComputeShellStress](/ADComputeShellStress.md).

## Stress divergence

Finally the residuals are computed and assembled in [ADStressDivergenceShell](/ADStressDivergenceShell.md) for both small and large strain scenarios. For small strain scenarios, the residual at each qp is computed by multiplying the stress tensor at that qp with the corresponding components of the B matrix. Additionally, for large strain scenarios, the old stress tensor is multiplied with corresponding components of the $B_{NL}$ matrix and that is also added to the residual at each qp. These two components for the residual are same as $K_{L} u$ and $K_{NL} u$ in equation 25 of [!cite](dvorkin1984continuum).

Note that there are quadrature points both along the plane as well as along the thickness of the element. The order of Gauss quadrature rule along the thickness is provided as input by the user to all the above mentioned objects.

## Inertia

This element's generalized inertia forces are obtained directly from the kinematic of the shell by computing the inertial forces component-wise. Inertia forces can be expressed in terms of a mass matrix as described in [!cite](Bolourchi1979).
The mass matrix takes the following form of a volume integral:

\begin{equation}
\mathbf{M} = \int_{V}^{} \rho \mathbf{H}^{T} \mathbf{H} dV
\end{equation}

where $\mathbf{H}$ is an element-wise matrix that contains the interpolation functions for displacement and rotational degrees of freedom. In the implementation, the volume integral is simplified, and it is assumed that the element thickness is constant. Similarly, one orientation matrix is used per element, that is, changes in orientation in a single element's geometry are neglected.

!bibtex bibliography
