# ViewFactorRayStudy

!syntax description /UserObjects/ViewFactorRayStudy

## Description

`ViewFactorRayStudy` computes view factors in a cavity enclosed by sidesets.
The view factors are computed by sending out rays from a sidesets along directions
determined by an angular quadrature. Rays are followed and traced through the cavity until
they hit any sideset that is not declared a "symmetry sideset". The view factor between
the sending and receiving sidesets is computed from the number of rays starting from one
and ending on the other side.

`ViewFactorRayStudy` uses ray tracing for computing view factors in two-dimensional and three-dimensional cavities. It does not impose
any restrictions on the geometry of the cavity; in particular it allows non-planar surfaces in radiative exchange, non-convex cavities, and obstruction
of view between two surfaces by a third surface.

## Theory

The central idea for computing view factors using ray tracing is to transform the integral over the target area into an integral over
angular direction (i.e., an integral over the field of view of any infinitesimal element on the starting surface). An angular quadrature
is used to numerically approximate the angular integral. To this end, the ray tracing module is used to follow rays along the directions
of the angular quadrature and determine which surface they intersect first. The ray is terminated on that surface and the contribution
to the view factor between the surface of origin and this surface is incremented.

The view factor, $F_{i,j}$, is defined as a double integral over patches $i$ and $j$. In 3D geometries, the view factor, $F_{i,j}$, is computed by:
\begin{equation}\label{eq:view_factor}
  F_{i,j} = \frac{1}{A_i \pi} \int_{A_i} \int_{A_j}  \frac{\cos \alpha_i \cos \alpha_j}{r_{i,j}^2}  dA_i dA_j
\end{equation}
where the integral is taken over points $\vec{r}_i$ on $A_i$ and $\vec{r}_j$ on $A_j$ on patches $i$ and $j$. For each combination of points $\vec{r}_i$ and $\vec{r}_j$, we define:

\begin{equation}
\begin{aligned}
    r_{i,j} &= \|\vec{r}_j - \vec{r}_i \|  \\
    \cos \alpha_i &=  \frac{\vec{n}_i \cdot \left( \vec{r}_j - \vec{r}_i \right) }{r} \\
    \cos \alpha_j &=  \frac{\vec{n}_j \cdot \left( \vec{r}_i - \vec{r}_j \right) }{r}
\end{aligned}
\end{equation}

where $\vec{n}_i$ is the normal vector on patch $i$ pointing into the cavity and $\vec{n}_j$ is the corresponding counterpart on patch $j$.

To facilitate the computation of view factors, we change the integration variable from $\vec{r}_j$ to angular direction $d \hat{\Omega}$.
The infinitesimal element $dA_j$ becomes:
\begin{equation}
    dA_j = \frac{r_{i,j}^2}{\cos \alpha_j} d \hat{\Omega}
\end{equation}
where $d \hat{\Omega} = \sin \alpha_i d\alpha_i d\omega$ and $\omega$ is the azimuthal angle of $\hat{\Omega}$ measured in a plane orthogonal to $\vec{n}_1$ with respect to an arbitrarily chosen vector in that plane.
The view factor is then computed by:
\begin{equation}
    F_{i,j} = \frac{1}{A_i \pi} \int_{A_i} \int_{\hat{\Omega} \in A_j}  \cos \alpha_1  ~dA_i d \hat{\Omega} = \frac{1}{A_i \pi} \int_{A_i} \int_{\hat{\Omega} \in A_j}  \hat{\Omega} \cdot \vec{n}_i~ dA_i d \hat{\Omega}
\end{equation}
where $\hat{\Omega} \in A_j$ are all angular directions (starting from $\vec{r}_i$) that intersect $A_j$ before intersecting any other radiation patch. We define $\mu_i = \cos \alpha_i$ and finally obtain:
\begin{equation}\label{eq:trafo_view_factor}
       F_{i,j} = \frac{1}{A_i \pi} \int_{A_i} \int_{\hat{\Omega} \in A_j} \mu_i~ dA_i d \hat{\Omega}
\end{equation}

Before discretizing [eq:trafo_view_factor], we rewrite it slightly by introducing the function $H_j\left(\vec{r},\hat{\Omega}\right)$ which is $1$ if
a ray starting from location $\vec{r}$ into direction $\hat{\Omega}$ makes its first intersection with surface $j$. Then [eq:trafo_view_factor] becomes

\begin{equation}\label{eq:trafo_view_factor_2}
 F_{i,j} = \frac{1}{A_i \pi} \int_{A_i} \int_{2 \pi^+} \mu_i H_j\left(\vec{r},\hat{\Omega}\right) ~dA_i d\hat{\Omega},
\end{equation}

The integral in [eq:trafo_view_factor_2] is discretized using a spatial quadrature over the area $i$ and an angular quadrature over $2 \pi^+$:

\begin{equation}\label{eq:trafo_trafo}
 F_{i,j} = \frac{1}{A_i \pi} \sum\limits_{l=1}^L  \sum\limits_{k=1}^K w_l \omega_k  \left(\hat{\Omega}_k\cdot \vec{n}_l\right) H_j\left(\vec{r}_l,\hat{\Omega}_k\right) ,
\end{equation}

where $l$ enumerates the spatial quadrature points, $k$ enumerates the angular directions, $\vec{r}_l$ is the location of spatial quadrature point $l$, $\vec{n}_l$ is the
normal at spatial quadrature point $l$, and $\hat{\Omega}_k$ is the k-th angular direction. The role of ray tracing in [eq:trafo_trafo] is to evaluate $H_j\left(\vec{r}_l,\hat{\Omega}_k\right)$.

`ViewFactorRayStudy` uses Gaussian product quadratures for integrals in space, and a half-range Gauss-Chebyshev quadrature in angle.
The selected angular quadrature is a half-range Gauss-Legendre-Chebyshev quadrature adopted from [!citep](WaltersLCQ) by first restricting the polar range to $0 < \mu_i < 1$ and rotating the angular directions so that the polar angle is measured with respect to $\vec{n}_i$ instead of $(0, 0, 1)$.

Symmetry surfaces require special treatment. The difference between surfaces that participate in the radiative transfer and symmetry surfaces is that
view factors are not computed for symmetry surfaces (i.e. if either surface $i$ or $j$ in $F_{i,j}$ is a symmetry surface, the view factor is not computed).
Rays neither start nor end on symmetry surfaces. Instead, rays are specularly reflected off of symmetry surfaces. This is facilitated by the ray tracing
module by using the [ReflectRayBC.md].

## Example Input syntax

!listing modules/heat_conduction/test/tests/view_factors/view_factor_2d.i
block=UserObjects

!syntax parameters /UserObjects/ViewFactorRayStudy

!syntax inputs /UserObjects/ViewFactorRayStudy

!syntax children /UserObjects/ViewFactorRayStudy

!bibtex bibliography
