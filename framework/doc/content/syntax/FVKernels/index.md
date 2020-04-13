# FVKernels System

For the finite volume method (FVM), `FVKernels` are the base class for `FVFluxKernel`, `FVElementalKernel`. These specialized objects satisfy the following tasks:

* `FVFluxKernel` represent numerical fluxes evaluate on the element faces. These   terms originate from applying Gauss' theorem.

* `FVElementalKernel` represent terms that do not contain a spatial derivative so that Gauss' theorem is not applied. These terms include time derivatives, externally imposed source terms, and reaction terms.

Currently, the `FVElementalKernel` category only contains kernels representing
time derivatives. Kernels representing externally imposed sources or reaction terms will be added in the near future.

## FVKernels block

`FVKernels` are added in the `FVKernels` block. An example is show in
the listing below. The `FVKernels` block in this example sets up a transient diffusion problem given by:

\begin{equation}
  \frac{\partial v}{\partial t} - \nabla \cdot D \nabla v = 0.
\end{equation}

The time derivative term corresponds to the kernel named `time`, while
the diffusion term is represented by the kernel named `diff`.

!listing test/tests/kernels/fv_simple_diffusion/fv_transient_diffusion.i
         block=FVKernels
         id=first_fv_kernel_example
         caption=Example of the FVKernels block in a [MOOSE] input file.

The `FVTimeKernel` in the example derives from `FVElementalKernel` so it's a volumetric contribution to the residual, while the `FVDiffusion` is a `FVFluxKernel` and it's a face contribution to the residual. The remaining MOOSE syntax is identical to finite element kernel objects.

* `variable` refers to the variable that this kernel is acting on (i.e. into which equation does the residual of this term go).

* `coeff` in kernel `diff` is a material property corresponding to the heat conduction or diffusion coefficient.

The next example is the `FVKernels` block for the solution of the one-dimensional Burgers' equation. The Burgers' equation for speed `v` is given by:

\begin{equation}
  \frac{\partial v}{\partial t} + \frac{1}{2}\frac{\partial }{\partial x} v^2 = 0.
\end{equation}

!listing test/tests/kernels/fv_burger/fv_burger.i
         block=FVKernels
         id=first_fv_kernel_example
         caption=Example of the FVKernels block in a [MOOSE] input file for solving one-dimensional Burgers' equation.

Note that the `FVBurger1D` kernel only works for one-dimensional problems. In this example, the exact same time derivative kernels as for the diffusion example is used, but the spatial derivative term is different.

Boundary conditions are not discussed in these examples. Check out the corresponding [syntax files](syntax/FVBCs/index.md)
for boundary conditions.

## FVKernel source code: FVDiffusion example

First, `FVFluxKernels` are discussed.
`FVFluxKernels` contribute face terms to the residual. The residual contribution is implemented
by overriding the `computeQpResidual` function. The examples presented here rely on automatic
differentiation for the Jacobian computation so that `computeQpJacobian` does not need to be
implemented.

In the FVM, one solves for the averages of the variables over the elements.
The values of the variables on the faces is unknown and must be computed
from the cell average values. This "interpolation" determines the accuracy
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

The term in parenthesis in the surface integral on the right hand side must be implemented
in the `FVKernel`. However, there is one more step before we can implement the kernel. We must determine how the values of $D$ and $\nabla v$ depend on the values of $D$ and $v$ on the right and
left side of the face $\partial \Omega$.
In this example, the following approximation is used:

\begin{equation}
    \left(-D(\vec{r}) \vec{n}\cdot \nabla v \right) \approx \frac{D(v_L,\vec{r}_L) + D(v_R,\vec{r}_R)}{2} \frac{v_R - v_L}{\|\vec{r}_R - \vec{r}_L\|}
\end{equation}

This is a central difference approximation of the gradient on the face that neglects cross
diffusion terms.

Now, the implementation of this numerical flux into `FVDiffusion::computeQpResidual`
is discussed.

* the kernel provides the left and right values of the variable $v$ as `_u_left[_qp]` and `_u_right[_qp]`

* the values of the material properties on the left is obtained by `_coeff_left(getADMaterialProperty<Real>("coeff"))` while
the right value is obtained by calling `getNeighborADMaterialProperty<Real>("coeff")`.

* geometric information about the left and right adjacent elements is available from the `face_info` object.

The implementation is then straight forward. The first line of the code computes `dudn` which corresponds to the term:

\begin{equation}
 \text{dudn} = \frac{v_R - v_L}{\|\vec{r}_R - \vec{r}_L\|}
\end{equation}

while the second line computes `k` corresponding to:

\begin{equation}
  \text{k} = \frac{D(v_L,\vec{r}_L) + D(v_R,\vec{r}_R)}{2} .
\end{equation}

The minus sign originates from the minus sign in the original expression. Flow from left to right is defined to be positive.

## FVKernel source code: FVMatAdvection example

In this example the advection term:

\begin{equation}
  \nabla \cdot \left( \vec{u} v \right)
\end{equation}

is discretized using upwinding. The velocity is denoted by $\vec{u}$ and $v$ could be a passive scalar quantity advected by the flow. Upwinding is a strategy that approximates the value of a variable on a face by using the value of upwind element (i.e. the element where the flow originates from).

!listing framework/src/fvkernels/FVDiffusion.C
         start=FVMatAdvection<compute_stage>
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
By convention, the normal $\vec{n}$ points from the left side to the right side.

The implementation is straight forward. In the constructor the left and right velocities are obtained as AD material properties templated on `RealVectorValue`. The average is computed
and stored in variable `v_avg`. The direction of the flow is determined using the inner product of `v_avg * _normal` and the residual is then computed using either the left value of $v$ given by `_u_left[_qp]` or the right value given by `_u_right[_qp]`.

## FVKernel source code: FVTimeKernel

This example demonstrates source code for an `FVElementalKernel`. `FVElementalKernel`
are volumetric terms. In this case, the kernel is `FVTimeKernel`.

!listing framework/src/fvkernels/FVTimeKernel.C
         start=FVTimeKernel<compute_stage>::computeQpResidual()
         end=template <>
         id=fv_time_code
         caption=Example source code for the finite volume time kernel.

This kernel implements the term:

\begin{equation}
  \frac{\partial v}{\partial t}
\end{equation}

The implementation is identical to the implementation of FEM kernels except that
the FVM does not require multiplication by the test function (one could think of FVM as having test function of one).

!syntax list /FVKernels objects=True actions=False subsystems=False
