# Pressure

!syntax description /BCs/Pressure

## Description

The boundary condition, `Pressure` applies a force to a mesh boundary in the magnitude
specified by the user.
A `component` of the normal vector to the mesh surface (0, 1, or 2 corresponding
to the $\hat{x}$, $\hat{y}$, and $\hat{z}$ vector components) is used to determine
the direction in which to apply the traction.
The boundary condition is typically applied to the displaced mesh.

The magnitude of the `Pressure` boundary condition can be specified as either a
scalar (use the input parameter `factor`, which defaults to 1.0), a `function` parameter, or a `Postprocessor`
name.  If more than one of these are given, they are multiplied by one another.

!alert note title=Can Be Created with the Pressure Action
A set of +`Pressure`+ boundary conditions applied to multiple variables in multiple
components can be defined with the [PressureAction](/BCs/Pressure/index.md).

### Jacobian

#### Cartesian

Let $N$ be the number of nodes on a finite element face.  Also, let $h_i$ be the shape function at node $i$.  Then the vector $f$, which has dimension $3N\times1$ for a 3D model, is defined as
\begin{equation}
  f = \int \gamma F^T n\;\; dA
\end{equation}
where $\gamma$ represents a scaling factor, $n$ is the normal vector, and
\begin{equation}
  F = \begin{bmatrix}
      {h_1}I & {h_2}I & ... & {h_N}I
      \end{bmatrix}
\end{equation}
where $I$ is the identity tensor.

To find the Jacobian, we take the variation of the term in the integral,
\begin{equation}
  \delta(\gamma F^T n) = \gamma(\delta(F^T)n + F^T\delta(n))
\end{equation}

We define $F_{,\alpha}$ as
\begin{equation}
  F_{,\alpha} = \begin{bmatrix}
                {h_{1,\alpha}}I & {h_{2,\alpha}}I & ... & {h_{N,\alpha}}I
                \end{bmatrix}
\end{equation}
where $\alpha$ may be either $\xi$ or $\eta$, the two parametric coordinates associated with the face of an element. $F_{,\alpha}$ has dimensions $3 \times 3n$.  This allows
\begin{equation}
\delta F = \begin{bmatrix}
           F_{,\xi}\delta\xi & F_{,\eta}\delta\eta
           \end{bmatrix}
\end{equation}
Since the values of the parametric coordinates are fixed according to the integration rule and do not vary, this term becomes zero.  We are left with $\gamma F^T\delta n$.

The normal vector $n$ is an outward unit vector at the integration points.  We define $b = q_{,\xi} \times q_{,\eta}$ where
\begin{equation}
q_{,\alpha} = F_{,\alpha}p
\end{equation}
defined at the integration points with
\begin{equation}
p^T = \begin{bmatrix}
      x^T_1 & x^T_2 & ... & x^T_n
      \end{bmatrix}
\end{equation}
$x_i = X_i + u_i$ where $X_i$ is the vector of coordinates for node $i$ and $u_i$ is the vector of displacments for node $i$.

\begin{equation}
\begin{aligned}
  \delta b = & \delta q_{,\xi} \times q_{,\eta} + q_{,\xi} \times \delta q_{,\eta} \\
  \delta q_{,\xi} = & F_{,\xi}\delta p + q_{,\xi\eta}\delta \eta \\
  \delta q_{,\eta} = & F_{,\eta}\delta p + q_{,\xi\eta}\delta \xi
\end{aligned}
\end{equation}
If $\delta\xi = \delta\eta = 0$,
\begin{equation}
\begin{aligned}
  \delta b = & F_{,\xi}\delta p \times q_{,\eta} + q_{,\xi} \times F_{,\eta}\delta p \\
           = & q_{,\xi} \times F_{,\eta}\delta p - q_{,\eta} \times F_{,\xi}\delta p \\
           = & (q_{,\xi} \times F_{,\eta} - q_{,\eta} \times F_{,\xi}) \delta p
\end{aligned}
\end{equation}

We are left with
\begin{equation}
  \delta(\gamma F^T n) = \frac{\gamma}{||b||}F^T(q_{,\xi} \times F_{,\eta} - q_{,\eta} \times F_{,\xi}) \delta p
\end{equation}

To take the cross product of a vector and a set of vectors in a matrix, we take the cross product of the vector with each vector in the matrix in turn.  The $j$th $3\times3$ submatrix of $F_{,\alpha}$ is $h_{j,\alpha}I$.  This gives
\begin{equation}
  (q_{,\xi} \times F_{,\eta} - q_{,\eta} \times F_{,\xi})_j =
  \begin{bmatrix}
  0 & -q_{,\xi(3)}h_{j,\eta}+q_{,\eta(3)}h_{j,\xi} & q_{,\xi(2)}h_{j,\eta}-q_{,\eta(2)}h_{j,\xi} \\
  q_{,\xi(3)}h_{j,\eta}-q_{,\eta(3)}h_{j,\xi} & 0 & -q_{,\xi(1)}h_{j,\eta}+q_{,\eta(1)}h_{j,\xi} \\
  -q_{,\xi(2)}h_{j,\eta}+q_{,\eta(2)}h_{j,\xi} & q_{,\xi(1)}h_{j,\eta}-q_{,\eta(1)}h_{j,\xi} & 0
  \end{bmatrix}
\end{equation}
The integrand of the $(i,j)$ $3\times3$ submatrix of the stiffness is
\begin{equation}
\frac{\gamma}{||b||}(F^T(q_{,\xi} \times F_{,\eta} - q_{,\eta} \times F_{,\xi}))_{ij} = \frac{\gamma}{||b||}h_i
\begin{bmatrix}
  0 & -q_{,\xi(3)}h_{j,\eta}+q_{,\eta(3)}h_{j,\xi} & q_{,\xi(2)}h_{j,\eta}-q_{,\eta(2)}h_{j,\xi} \\
  q_{,\xi(3)}h_{j,\eta}-q_{,\eta(3)}h_{j,\xi} & 0 & -q_{,\xi(1)}h_{j,\eta}+q_{,\eta(1)}h_{j,\xi} \\
  -q_{,\xi(2)}h_{j,\eta}+q_{,\eta(2)}h_{j,\xi} & q_{,\xi(1)}h_{j,\eta}-q_{,\eta(1)}h_{j,\xi} & 0
\end{bmatrix}
\end{equation}

#### Spherical symmetry

For a problem using spherical symmetry, the Jacobian is much simpler.  Here we have
\begin{equation}
f = \int \gamma F^Tn \;\; dA = \int\int \gamma F^T n r^2 \sin \phi \;\; d\phi \; d\theta = 4\pi\gamma F^T n r^2
\end{equation}
with $r = X + u$.
Here, $n$ is not a function of the displacements.  The variation is
\begin{equation}
8 \pi \gamma F^T n r\delta r
\end{equation}

#### Axisymmetry

For 1D axisymmetry, we have
\begin{equation}
f = \int \gamma F^Tn \;\; dA = \int \gamma F^T nr \;\; d\theta = 2\pi \gamma F^T n r
\end{equation}
with $r = X + u$ and a unit height.
Here, $n$ is not a function of the displacements.  The variation is
\begin{equation}
2 \pi \gamma F^T n \delta r
\end{equation}

For 2D axisymmetry, we have
\begin{equation}
f = \int \gamma F^Tn \;\; dA = \int \int \gamma F^T n r \;\; d\theta \; dz = \int 2\pi\gamma F^T n r \;\; dz
\end{equation}
with $r = X + u$.  However, both $n$ and $r$ depend on the displacements.  Thus, we have
\begin{equation}
\delta(2\pi\gamma F^T n r) = 2 \pi \gamma F^T (r \delta n + n \delta r)
\end{equation}


## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/1D_spherical/finiteStrain_1DSphere_hollow.i block=BCs/outerPressure

!syntax parameters /BCs/Pressure

!syntax inputs /BCs/Pressure

!syntax children /BCs/Pressure
