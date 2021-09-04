# FVKernels System

For an overview of MOOSE FV please see [/fv_design.md].

For the finite volume method (FVM), `FVKernels` are the base class for `FVFluxKernel`, `FVElementalKernel`. These specialized objects satisfy the following tasks:

* `FVFluxKernel` represents numerical fluxes evaluate on the element faces.
  These terms originate from applying Gauss' divergence theorem.

* `FVElementalKernel` represents terms that do not contain a spatial
  derivative so that Gauss' theorem cannot be applied. These terms include
  time derivatives, externally imposed source terms, and reaction terms.

Note: Currently, the `FVElementalKernel` category only contains kernels
(subclasses) representing time derivatives. Kernels representing externally
imposed sources or reaction terms will be added in the near future.

!alert note
In the documentation that follows, we will use '-' and '+' to represent
different sides of a face. This is purely notation. In the MOOSE code base, the
'-' side is represented with an `_elem` suffix and the '+' side is represented
with a `_neighbor` suffix. We could just as well have chosen `_left` and
`_right`, or `_1` and `_2`, or `_minus` and `_plus`, but for consistency with previous MOOSE framework
code such as discontinuous Galerkin kernels and node-face constraints, we have
elected to go with the `_elem` and `_neighbor` suffixes.

## FVKernels block

FVM kernels are added to simulation input files in the `FVKernels` block.  The
`FVKernels` block in the example below sets up a transient diffusion problem
defined by the equation:

\begin{equation}
  \frac{\partial v}{\partial t} - \nabla \cdot D \nabla v = 0.
\end{equation}

The time derivative term corresponds to the kernel named `time`, while
the diffusion term is represented by the kernel named `diff`.

!listing test/tests/fvkernels/fv_simple_diffusion/transient.i
         block=FVKernels
         id=first_fv_kernel_example
         caption=Example of the FVKernels block in a MOOSE input file.

The `FVTimeKernel` in the example derives from `FVElementalKernel` so it's a
volumetric contribution to the residual, while the `FVDiffusion` kernel is an
`FVFluxKernel` and it's a face contribution to the residual. The remaining
MOOSE syntax is what you would expect to see in finite element kernel objects:

* `variable` refers to the variable that this kernel is acting on (i.e. into
  which equation does the residual of this term go).  This must be a
  finite-volume variable (defined with `fv = true`) for all FVM kernels.

* `coeff` in kernel `diff` is a material property corresponding to the heat conduction or diffusion coefficient.

The next example shows an `FVKernels` block that solves the one-dimensional
Burgers' equation. The Burgers' equation for speed `v` is given by:

\begin{equation}
  \frac{\partial v}{\partial t} + \frac{1}{2}\frac{\partial }{\partial x} v^2 = 0.
\end{equation}

!listing test/tests/fvkernels/fv_burgers/fv_burgers.i
         block=FVKernels
         id=second_fv_kernel_example
         caption=Example of the FVKernels block in a MOOSE input file for solving one-dimensional Burgers' equation.

Note that the `FVBurgers1D` kernel only works for one-dimensional problems. In
this example, the exact same time derivative kernels as for the diffusion
example is used, but the spatial derivative term is different.

Boundary conditions are not discussed in these examples. Look at
[syntax files](syntax/FVBCs/index.md) for details about boundary conditions.

!syntax list /FVKernels objects=True actions=False subsystems=False

## FVKernel source code: FVDiffusion example

First, `FVFluxKernels` are discussed.  `FVFluxKernels` are used to calculate
numerical flux contributions from face (surface integral) terms to the
residual. The residual contribution is implemented by overriding the
`computeQpResidual` function.

In the FVM, one solves for the averages of the variables over each element.
The values of the variables on the faces are unknown and must be computed
from the cell average values. This interpolation/reconstruction determines the accuracy
of the FVM.
The discussion is based on the example of `FVDiffusion` that discretizes the diffusion term using a central difference approximation.

!listing framework/src/fvkernels/FVDiffusion.C
         start=template
         end=registerADMooseObject("MooseApp", FVMatAdvection);
         id=fv_diffusion_code
         caption=Example source code for a finite volume kernel discretizing the diffusion term using a central difference.

The kernel `FVDiffusion` discretizes the diffusion term $-\nabla \cdot D(v,\vec{r}) \nabla v$.
Integrating over the extend of an element and using Gauss' theorem leads to:

\begin{equation}
-  \int_{\Omega} \nabla \cdot D(v,\vec{r}) \nabla v dV =  \int_{\partial \Omega} \left(-D(v, \vec{r}) \vec{n}\cdot \nabla v \right) dS.
\end{equation}

The term in parenthesis in the surface integral on the right hand side must be
implemented in the `FVKernel`. However, there is one more step before we can
implement the kernel. We must determine how the values of $D$ and $\nabla v$
depend on the values of $D$ and $v$ on the '+' and '-' side of the face
$\partial \Omega$.  In this example, the following approximation is used:

\begin{equation}
    \left(-D(\vec{r}) \vec{n}\cdot \nabla v \right) \approx \frac{D(v_L,\vec{r}_L) + D(v_R,\vec{r}_R)}{2} \frac{v_R - v_L}{\|\vec{r}_R - \vec{r}_L\|}
\end{equation}

This is a central difference approximation of the gradient on the face that neglects cross
diffusion terms.

Now, the implementation of this numerical flux into `FVDiffusion::computeQpResidual`
is discussed.

* the kernel provides the '-' and '+' values of the variable $v$ as `_u_elem[_qp]` and `_u_neighbor[_qp]`

* the values of the material properties on the '-' side of the face is obtained by `_coeff_elem(getADMaterialProperty<Real>("coeff"))` while
the '+' side value is obtained by calling `getNeighborADMaterialProperty<Real>("coeff")`.

* geometric information about the '-' and '+' adjacent elements is available from the `face_info` object.

The implementation is then straight forward. The first line of the code computes `dudn` which corresponds to the term:

\begin{equation}
 \text{dudn} = \frac{v_R - v_L}{\|\vec{r}_R - \vec{r}_L\|}
\end{equation}

while the second line computes `k` corresponding to:

\begin{equation}
  \text{k} = \frac{D(v_L,\vec{r}_L) + D(v_R,\vec{r}_R)}{2} .
\end{equation}

The minus sign originates from the minus sign in the original expression. Flow from '-' to '+ is defined to be positive.

## FVKernel source code: FVMatAdvection example

In this example the advection term:

\begin{equation}
  \nabla \cdot \left( \vec{u} v \right)
\end{equation}

is discretized using upwinding. The velocity is denoted by $\vec{u}$ and $v$
represents a passive scalar quantity advected by the flow. Upwinding is a
strategy that approximates the value of a variable on a face by taking the
value from the upwind element (i.e. the element where the flow originates from).

!listing framework/src/fvkernels/FVDiffusion.C
         start=FVMatAdvection::
         end=" "
         id=fv_mat_advection_code
         caption=Example source code for a finite volume kernel discretizing advection of a passive scalar.

Integrating the advection term over the element and using Gauss' theorem leads to:

\begin{equation}
   \int_{\Omega}   \nabla \cdot \left( \vec{u} v \right) dV =
   \int_{\partial \Omega} \left(\vec{n} \cdot \vec{u} v \right) dS.
\end{equation}

This term in parenthesis on the right hand side is approximated using upwinding:

\begin{equation}
   \vec{n} \cdot \vec{u} v \approx  \tilde{\vec{u}}\cdot \vec{n}
   \tilde{v}
   ,
\end{equation}

where

\begin{equation}
   \tilde{\vec{u}} = \frac{1}{2} \left( \vec{u}_L + \vec{u}_R \right)
\end{equation}

and  $\tilde{v} = v_L$ if $\tilde{\vec{u}} \cdot \vec{n} > 0$ and $\tilde{v} = v_R$ otherwise.
By convention, the normal $\vec{n}$ points from the '-' side to the '+' side.

The implementation is straight forward. In the constructor the '-' and '+'
velocities are obtained as `RealVectorValue` material properties. The average
is computed and stored in variable `v_avg`. The direction of the flow is
determined using the inner product of `v_avg * _normal` and the residual is
then computed using either the '-' value of $v$ given by `_u_elem[_qp]` or
the '+' value given by `_u_neighbor [_qp]`.

## FVKernel source code: FVTimeKernel

This example demonstrates source code for an `FVElementalKernel`. `FVElementalKernel`
are volumetric terms. In this case, the kernel is `FVTimeKernel`.

!listing framework/src/fvkernels/FVTimeKernel.C
         start=FVTimeKernel::computeQpResidual()
         end=template <>
         id=fv_time_code
         caption=Example source code for the finite volume time kernel.

This kernel implements the term:

\begin{equation}
  \frac{\partial v}{\partial t}
\end{equation}

The implementation is identical to the implementation of FEM kernels except that
the FVM does not require multiplication by the test function.
