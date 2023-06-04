# Finite Element Method (FEM)

!---

## Polynomial Fitting

To introduce the concept of FEM, consider a polynomial fitting exercise. When fitting a polynomial
there is a known set of points as well as a set of coefficients that are unkown for a function
that has the form:

!equation
f(x) = a + bx + cx^2 + \dots,

where $a$, $b$ and $c$ are scalar coefficients and $1$, $x$, $x^2$ are "basis functions". Thus, the
problem is to find $a$, $b$, $c$, etc. such that $f(x)$ passes through the points given.

More generally,

!equation
f(x) = \sum_{i=0}^d c_i x^i,

where the $c_i$ are coefficients to be determined. $f(x)$ is unique and interpolary if $d+1$ is the
same as the number of points needed to fit. This defines a linear system that must be solved to
find the coefficients.

!---

## Polynomial Example

Define a set of points:

!equation
(x_1, y_1) = (1,4) \\
(x_2, y_2) = (3,1) \\
(x_3, y_3) = (4,2)

Substitute $(x_i, y_i)$ data into the model:

!equation
y_i = a + bx_i + cx_i^2, i=1,2,3.

This leads to the following linear system for $a$, $b$, and $c$:

!equation
\begin{bmatrix}
  1 & 1 &  1 \\
  1 & 3 &  9 \\
  1 & 4 & 16
\end{bmatrix}
\begin{bmatrix}
  a \\
  b \\
  c
\end{bmatrix}
=
\begin{bmatrix}
  4 \\
  1 \\
  2
\end{bmatrix}

!---

Solving for the coefficients results in:

!equation
\begin{bmatrix}
  a \\
  b \\
  c
\end{bmatrix}
=
\begin{bmatrix}
  8 \\
  \frac{29}{6} \\
  \frac{5}{6}
\end{bmatrix}

These coefficients define the solution function:

!equation
f(x) = 8 - \frac{29}{6} x + \frac{5}{6} x^2

!media darcy_thermo_mech/fem_example.png style=width:60%;margin-left:auto;margin-right:auto;display:block;

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

## Simplified FEM

FEM is a method for numerically approximating the solution to [!ac](PDEs).

FEM finds a solution function that is made up of "shape functions" multiplied by coefficients and
added together, just like in polynomial fitting, except the functions are not typically as simple
(although they can be).

The Galerkin Finite Element method is different from finite difference and finite volume methods
because it finds a piecewise continuous function which is an approximate solution to the governing
PDEs.

Just as in polynomial fitting you can evaluate a finite element solution anywhere in the domain.

FEM is widely applicable for a large range of PDEs and domains.

It is supported by a rich mathematical theory with proofs about accuracy, stability, convergence and
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

!---

## Integration by Parts and Divergence Theorem

Suppose $\varphi$ is a scalar function, $\vec{v}$ is a vector function, and both are continuously
differentialable functions, then the product rule states:

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
!col! width=20%
!listing test/tests/kernels/2d_diffusion/2d_diffusion_neumannbc_test.i block=Kernels remove=Kernels/active
!col-end!

!col! width=1%
$\quad$
!col-end!

!col! width=20%
!listing test/tests/kernels/2d_diffusion/2d_diffusion_neumannbc_test.i block=BCs remove=BCs/active BCs/left
!col-end!

!col! width=1%
$\quad$
!col-end!

!col! width=20%
!listing test/tests/dgkernels/1d_advection_dg/1d_advection_dg.i block=Kernels remove=Kernels/time_u
!col-end!

!col! width=1%
$\quad$
!col-end!

!col! width=20%
!listing test/tests/bcs/nodal_normals/circle_tris.i block=Kernels remove=Kernels/diff
!col-end!

!row-end!

!style-end!
