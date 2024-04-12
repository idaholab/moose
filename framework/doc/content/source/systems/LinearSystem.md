# LinearSystem

## Solving nonlinear problems with customizable linearization in MOOSE

In MOOSE, a `LinearSystem` denotes a linear algebraic equation system that
arises from the spatial discretization of a linearized partial differential equation.
Unlike [NonlinearSystem.md], where Newton's method is used to linearize a nonlinear equation,
the `LinearSystem` allows the user to customize how the system is
linearized using objects that contribute to a system matrix and right hand side.
The simplest case is a Picard-style approach where coupled nonlinear terms are
linearized by evaluating with known solution states. For better clarity an example is provided below.

!alert note
Currently, `LinearSystem` with a Picard-style solution approach is only supported with the
finite volume spatial discretization.

## Picard-style diffusion problem in MOOSE using a LinearSystem

Considering that at the time this documentation is written only finite volume simulations
utilize this system, this example presents a nonlinear 1D diffusion problem with a fixed source term:

!equation
-\nabla \cdot (D(u) \nabla u) = 0,~ \vec{r}\in\Omega, \mathrm{~where~}\Omega\in[0,1]

with the following Dirichlet boundary conditions on the left and right boundaries of the domain:

!equation
u(\vec{r}_L) = u_L

!equation
u(\vec{r}_R) = u_R

Let's split the domain into 3 cells with the following cell widths: $|d|=0.\dot{3}$.
Considering that the diffusion coefficient ($D(u)$) is a function of the solution, we need to linearize the problem.
In this example we use a Picard-style linearization where the $n$-th iterate is denoted by: $u^n$.
This leads to the following equation:

-\nabla \cdot (D(u^{n-1}) \nabla u^n) = S,

To derive the discretized form of the diffusion term, we first integrate it over the volume of cell $i\in[1,2,3]$
(denoted by $\Omega_i$):

!equation
\int\limits_{\Omega_i}-\nabla \cdot (D(u^{n-1}) \nabla u^n)dV = \int\limits_{\partial \Omega_i} -D(u^{n-1}) \nabla u^n \cdot \vec{n} dS

where the divergence theorem was employed to derive a surface integral with $\vec{n}$ being the surface normal.
Now, employing a cell-centered finite volume approximation, we assume that the solution field is cell-wise
constant so we have 3 unknowns: $u_1,~u_2$ and $u_3$ often referred to as degrees of freedom.
Furthermore, $\nabla u^n \vec{n}$ over the faces between cells can be approximated as:

!equation
\nabla u^n \cdot \vec{n} \approx \frac{u^n_{i+1}-u^n_i}{|d|}~,

while on boundaries it is approximated as follows:

!equation
\nabla u^n \cdot \vec{n}_b \approx 2\frac{u_b-u^n_i}{|d|}~.

One approach to express the diffusion coefficient on the face is to use linear interpolation
using the diffusion coefficients at the cell centers:

!equation
D(u^n)_{f,i+{1/2}} = \frac{D(u^n_i)+D(u^n_{i+1})}{2},

whereas the diffusion coefficients on the Dirichlet boundaries can just be evaluated at the boundary value:
$D_b = D(u_b)$. With this in mind, we can write down three equations for the three cells:

!equation
- D_L 2\frac{u_L-u^n_0}{|d|} - D(u^{n-1})_{f,3/2} \frac{u^n_2-u^n_1}{|d|} = 0,

!equation
- D(u^{n-1})_{f,3/2} \frac{u^n_1-u^n_2}{|d|} - D(u^{n-1})_{f,5/2} \frac{u^n_3-u^n_2}{|d|} = 0,

!equation
- D(u^{n-1})_{f,5/2} \frac{u^n_3-u^n_2}{|d|} - D_R 2\frac{u_R-u^n_3}{|d|} = 0.

These provide a linear algebraic equation system for the following three variables: $u^n_1,~u^n_2$ and $u^n_3$.
The algebraic equation system in a matrix notation can be expressed as:

!equation
\mathbf{A}(\vec{u}^{n-1})\vec{u}^n = \vec{b},

where $\mathbf{A}(\vec{u^{n-1}})$ is referred to as system matrix (often just matrix) and $\vec{b}$ is the
right hand side vector. In this specific example the system matrix is:

!equation
\mathbf{A}(\vec{u}^{n-1})=
\begin{bmatrix}
\frac{2D_L}{|d|} + \frac{D(u^{n-1})_{f,3/2}}{|d|} & \frac{-D(u^{n-1})_{f,3/2}}{|d|} & 0 \\
\frac{-D(u^{n-1})_{f,3/2}}{|d|} & \frac{D(u^{n-1})_{f,3/2}}{|d|} + \frac{D(u^{n-1})_{f,5/2}}{|d|} & \frac{-D(u^{n-1})_{f,5/2}}{|d|} \\
0 & \frac{-D(u^{n-1})_{f,5/2}}{|d|} & \frac{D(u^{n-1})_{f,5/2}}{|d|} + \frac{2D_R}{|d|} \\
\end{bmatrix},

while the right hand side vector can be expressed as:

!equation
\vec{b}=
\begin{bmatrix}
\frac{2D_Lu_L}{|d|} \\
0 \\
\frac{2D_Ru_R}{|d|}
\end{bmatrix}.

This equation can be solved for $\vec{u}^{n}$ which can be used to update the diffusion coefficient and
recompute the system matrix to solve the equation for the next iterate, $\vec{u}^{n+1}$ until this process converges to $\vec{u}$ with a given tolerance.

## Populating the system in LinearSystem

At the moment the user can implement new objects derived from [linear finite volume kernels](LinearFVKernels/index.md) and linear [finite volume boundary conditions](LinearFVBCs/index.md) to provide the entries in
system matrix $mathbf{A}$ and right hand side vector $\vec{b}$.
