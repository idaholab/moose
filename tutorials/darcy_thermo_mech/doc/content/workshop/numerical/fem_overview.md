# Finite Element Method (FEM)

!---

## Function Approximation

To introduce the concept of FEM, consider a polynomial regression exercise. We can
determine a polynomial function in a:

- +Discrete sense+: Using sampled locations with associated values, we want our polynomial function to be as close to the given data as possible.
- +Continuous sense+: Using a complicated function, we want to have our polynomial function as close to the given function as possible.


Let us take the polynomial in the following form:

!equation
f(x) = \sum_{i=0}^d c_i x^i

where $c_i$ are scalar coefficients (expansion coefficients) and $x^i$ are "basis functions". Thus, the
problem is to find $c_i$ such that $f(x)$ is closest the the given points or a
given function.

!---

## Polynomial Example (Discrete)

!row!
!col! width=49%

Define a set of points (Let's pick the points using $sin(x)$):

!equation
(x_1, y_1) = (0.25,0.247) \\
(x_2, y_2) = (0.50,0.479) \\
(x_3, y_3) = (0.75,0.682) \\
(x_4, y_4) = (1.00,0.841)


Substitute $(x_i, y_i)$ data into the polynomial model:

!equation
c_0 + c_1 x_i + c_2 x_i^2 = y_i,\quad i=1,..,4.

This leads to the following linear system for $c_i$:

!equation
\mathbf{A}
\vec{c}
=
\vec{y}

where

!equation
\mathbf{A} = \begin{bmatrix}
  1 & 0.25 & 0.25^2 \\
  1 & 0.50 & 0.50^2 \\
  1 & 0.75 & 0.75^2 \\
  1 & 1.00 & 1.00^2
\end{bmatrix}, \quad
\vec{c} = \begin{bmatrix}
  c_0 \\
  c_1 \\
  c_2 \\
  c_3
\end{bmatrix} \quad
\vec{y}\begin{bmatrix}
  0.247 \\
  0.479 \\
  0.682 \\
  0.841
\end{bmatrix}

!col-end!

!col! width=49%

!alert! note title=Solving this problem

We want to get the function closest to the points.
Distance in this case can be expressed by the discrete
$L^2$ norm:

!equation
R = ||\mathbf{A}\vec{c}-\vec{y}||^2_2,\quad \text{where} \quad
||\vec{v}||^2_2 = \vec{v}^T \vec{v},

We want to minimize the norm of the squared distances ("least squares"):

!equation
\frac{\partial R}{\partial \vec{c}} = \mathbf{A}^T (\mathbf{A}\vec{c} - \vec{y}) = 0

which requires the solution of this problem:

!equation
\mathbf{A}^T \mathbf{A}\vec{c} = \mathbf{A}^T\vec{y}

which results in:

!equation
\vec{c} =
\begin{bmatrix}
  -0.024 \\
  1.155 \\
  -0.289
\end{bmatrix}

!alert-end!

!col-end!

!row-end!

!---

These coefficients define the solution function:

!equation
f(x) = 8 - \frac{29}{6} x + \frac{5}{6} x^2

!plot scatter data=[{'x':[0.25,0.5,0.75,1.0], 'y':[0.247,0.479,0.682,0.841], 'name':'Data', 'mode':'markers', 'color':'red', 'marker':{'size':14}}, {'x':[0.0,0.025,0.05,0.075,0.1,0.125,0.15,0.175,0.2,0.225,0.25,0.275,0.3,0.325,0.35,0.375,0.4,0.425,0.45,0.475,0.5,0.525,0.55,0.575,0.6,0.625,0.65,0.675,0.7,0.725,0.75,0.775,0.8,0.825,0.85,0.875,0.9,0.925,0.95,0.975,1], 'y':[-0.023855457,0.004831882,0.033158274,0.061123719,0.088728218,0.11597177,0.142854375,0.169376033,0.195536745,0.221336509,0.246775327,0.271853199,0.296570123,0.320926101,0.344921131,0.368555216,0.391828353,0.414740543,0.437291787,0.459482084,0.481311434,0.502779838,0.523887295,0.544633804,0.565019368,0.585043984,0.604707654,0.624010376,0.642952152,0.661532982,0.679752864,0.6976118,0.715109789,0.732246831,0.749022927,0.765438075,0.781492277,0.797185532,0.81251784,0.827489202,0.842099617], 'name':'f(x)'}]
             layout={'margin':{'t':5,'b':5}}

!style halign=center fontsize=115%
+The solution is the function, *not* the coefficients.+

!---

The coefficients are meaningless, they are just numbers used to define a function.

The solution is *not* the coefficients, but rather the *function* created when they are
multiplied by their respective basis functions and summed.

The function $f(x)$ does go through the points given, *but it is also defined everywhere in
between*.

$f(x)$ can be evaluated at the point $x=2$, for example, by computing:

!equation
f(2) = \sum_{i=0}^2 c_i 2^i = \sum_{i=0}^2 c_i g_i(2),

where the $c_i$ correspond to the coefficients in the solution vector, and the $g_i$ are the
respective functions.

!---

## FEM can be used to solve both linear and nonlinear PDEs

FEM is a method for numerically approximating the solution to [!ac](PDEs).
FEM is widely applicable for a large range of PDEs and domains.

Example PDEs:
Have you seen them before? Are they linear/nonlinear? Coupled?

!equation id=diffusion
-\nabla \cdot \nabla u = q

!equation id=ns_momentum
\dfrac{\partial u_x}{\partial t} -\nabla\cdot \mu \nabla u_x + \vec{u} \cdot \nabla u_x = 0

!equation id=reactor_physics
C \dfrac{\partial T}{\partial t} -\nabla \cdot k(T) \nabla T = q(\psi) \\
\dfrac{1}{v} \dfrac{\partial \psi}{\partial t} + \Omega \cdot \nabla \psi + \Sigma(T) \psi = Q(\psi, T)

!---

## FEM is a general method to discretize these equations

FEM finds a solution function that is made up of "shape functions" multiplied by coefficients and
added together, just like in polynomial fitting, except the functions are not typically as simple
(although they can be).

The Galerkin Finite Element method is different from finite difference and finite volume methods
because it finds a piecewise continuous function which is an approximate solution to the governing
PDEs.

FEM provides an approximate solution. The true solution can only be represented as well as the shape
function basis can represent it!

FEM is supported by a rich mathematical theory with proofs about accuracy, stability, convergence and
solution uniqueness.

!---

## Weak Form

Using FEM to find the solution to a PDE starts with forming a "weighted residual" or "variational
statement" or "weak form", this processes if referred to here as generating a weak form.

The weak form provides flexibility, both mathematically and numerically and it is needed
by MOOSE to solve a problem.

Generating a weak form generally involves these steps:

1.  Write down strong form of PDE.
1.  Rearrange terms so that zero is on the right of the equals sign.
1.  Multiply the whole equation by a "test" function $\psi$.
1.  Integrate the whole equation over the domain $\Omega$.
1.  Integrate by parts and use the divergence theorem to get the desired derivative order on your
    functions and simultaneously generate boundary integrals.

!alert note title=Exercise
Obtain the weak form for the equations listed on the previous slide
and the shape functions.

!---

## Integration by Parts and Divergence Theorem

Suppose $\varphi$ is a scalar function, $\vec{v}$ is a vector function, and both are continuously
differentiable functions, then the product rule states:

!equation
\nabla\cdot(\varphi\vec{v}) = \varphi(\nabla\cdot\vec{v}) + \vec{v}\cdot(\nabla \varphi)

The function can be integrated over the volume $V$ and rearranged as:

!equation id=integrate_by_parts
\int \varphi(\nabla\cdot\vec{v})\,dV = \int \nabla\cdot(\varphi\vec{v})\,dV - \int \vec{v}\cdot(\nabla\varphi)\,dV

The divergence theorem transforms a volume integral into a surface integral on surface $s$:

!equation id=divergence_theorem
\int \nabla \cdot (\varphi\vec{v})\,\text{d}V = \int \varphi\vec{v}\cdot\hat{n}\,\text{d}s,

where $\hat{n}$ is the outward normal vector for surface $s$. Combining [integrate_by_parts] and
[divergence_theorem] yield:

!equation id=int_and_div
\boxed{\int \varphi (\nabla\cdot\vec{v})\,dV = \int \varphi\vec{v}\cdot\hat{n}\,ds - \int\vec{v}\cdot(\nabla\varphi)\,dV}

!---

## Example: Advection-Diffusion

+(1)+ Write the strong form of the equation:

!equation
-\nabla\cdot k\nabla u + \vec{\beta} \cdot \nabla u = f

+(2)+ Rearrange to get zero on the right-hand side:

!equation
-\nabla\cdot k\nabla u + \vec{\beta} \cdot \nabla u - f = 0

+(3)+ Multiply by the test function $\psi$:

!equation
-\psi \left(\nabla\cdot k\nabla u\right) + \psi\left(\vec{\beta} \cdot \nabla u\right) - \psi f = 0

!---

+(4)+ Integrate over the domain $\Omega$:

!equation
{-\int_\Omega\psi \left(\nabla\cdot k\nabla u\right)} + \int_\Omega\psi\left(\vec{\beta} \cdot \nabla u\right) - \displaystyle\int_\Omega\psi f = 0

+(5)+ Integrate by parts and apply the divergence theorem, by using [int_and_div] on the left-most
      term of the PDE:

!equation
\int_\Omega\nabla\psi\cdot k\nabla u -
\int_{\partial\Omega} \psi \left(k\nabla u \cdot \hat{n}\right) +
\int_\Omega\psi\left(\vec{\beta} \cdot \nabla u\right) - \int_\Omega\psi f = 0

Write in inner product notation. Each term of the equation will inherit from an existing MOOSE type as shown below.

!equation id=example_weak_form
\underbrace{\left(\nabla\psi, k\nabla u \right)}_{Kernel} -
\underbrace{\langle\psi, k\nabla u\cdot \hat{n} \rangle}_{BoundaryCondition} +
\underbrace{\left(\psi, \vec{\beta} \cdot \nabla u\right)}_{Kernel} -
\underbrace{\left(\psi, f\right)}_{Kernel} = 0

!---

## Corresponding MOOSE input file blocks

!style! fontsize=140%

!equation
\underbrace{\left(\nabla\psi, k\nabla u \right)}_{Kernel} -
\underbrace{\langle\psi, k\nabla u\cdot \hat{n} \rangle}_{BoundaryCondition} +
\underbrace{\left(\psi, \vec{\beta} \cdot \nabla u\right)}_{Kernel} -
\underbrace{\left(\psi, f\right)}_{Kernel} = 0

!style-end!

!style! fontsize=60%

!row!
!col! width=15%
!listing test/tests/kernels/2d_diffusion/2d_diffusion_neumannbc_test.i block=Kernels remove=Kernels/active
!col-end!

!col! width=1%
$\quad$
!col-end!

!col! width=15%
!listing test/tests/kernels/2d_diffusion/2d_diffusion_neumannbc_test.i block=BCs remove=BCs/active BCs/left
!col-end!

!col! width=1%
$\quad$
!col-end!

!col! width=15%
!listing test/tests/dgkernels/1d_advection_dg/1d_advection_dg.i block=Kernels remove=Kernels/time_u
!col-end!

!col! width=1%
$\quad$
!col-end!

!col! width=15%
!listing test/tests/bcs/nodal_normals/circle_tris.i block=Kernels remove=Kernels/diff
!col-end!

!row-end!

!style-end!
