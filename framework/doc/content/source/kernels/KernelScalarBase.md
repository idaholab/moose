# KernelScalarBase

The `KernelScalarBase` is part of the scalar augmentation system to complement the [ScalarKernel](syntax/ScalarKernels/index.md) class. Its principal purpose is to add standard
quadrature loops and assembly routines to handle the contributions from a single added
scalar variable to a [Kernel](syntax/Kernels/index.md) class, including the entire row of the Jacobian.
These routines will assist with representing weak form terms in a partial differential equation involving
scalar variables integrated over the interior of a domain. As usual, this piece of physics is referred to
as the "residual" and is evaluated at integration quadrature points within that domain. To implement your own physics in MOOSE, you create your own kernel by subclassing the MOOSE `KernelScalarBase` class.

The Kernel scalar augmentation system supports the use of [!ac](AD) for residual calculations, as such
there are two options for creating field-scalar coupling objects:
`KernelScalarBase` and `ADKernelScalarBase`. To further understand
automatic differentiation, please refer to the [automatic_differentiation/index.md] page for more
information.

Developers should read all sections; users can find [#KSB-parameters] described at the bottom.

## Creation of Kernel Scalar Coupling Classes id=KSB-coupling

Each `Kernel` object has a focus field variable or spatial variable; its job is to contribute to the
residual as well as the row of the Jacobian matrix. Herein, as in the source code of `Kernels`, this
spatial variable will be called `_var`. In a coupled (multi-phyics) weak form, all domain integral terms
containing the test function of `_var` are potential candidates for `Kernel` contributions.

The philosphy of the scalar augmentation class `KernelScalarBase` is to add a focus scalar variable
referred to as `_kappa` to the `Kernel` object so that all terms in the coupled weak form that involve
`_var`, `_kappa`, and/or their test functions can be assembled in one or multiple class instances.
This philosophy is similar to how the lower dimensional variable `_lambda` is added to the element faces
of `DGKernel` and `IntegratedBC` objects associated with the hybrid finite element method (HFEM). Documentation for that approach can be found [HFEMDiffusion](source/dgkernels/HFEMDiffusion.md) and [HFEMDirichletBC](source/bcs/HFEMDirichletBC.md) along with the base classes `DGLowerDKernel` and `LowerDIntegratedBC`.

In a `KernelScalarBase` subclass, a naming scheme is established for the quadrature point methods of the two
variable types: methods contributing to the test function of `_kappa` have "Scalar" near the front
and methods contributing to the trial function of scalar variables in the Jacobian have "Scalar"
at the end. The `computeScalarQpResidual()` function +should+ be overridden (see [#KSB-parameters]
for cases where the scalar should be suppressed). The `computeQpResidual()` function +must+ be
overridden as usual for `Kernels`, although it may return zero.

For non-AD objects, several contributions to the Jacobian matrix can be optionally overridden for use in
Newton-type nonlinear solvers. As mentioned later, the developer should choose and document which terms
(rows) of the residual and terms (rows and columns) of the Jacobian will be attributed to an instance of
the developed class. These choices can be motivated by whether some terms in the weak form can be or have
already been implemented within other MOOSE classes.

- `computeQpJacobian()`: Jacobian component d-`_var`-residual / d-`_var`
- `computeQpOffDiagJacobian(jvar_num)`: off-diagonal Jacobian component d-`_var`-residual / d-`jvar`
- `computeQpOffDiagJacobianScalar(svar_num)`: off-diagonal Jacobian component d-`_var`-residual / d-`svar`
- `computeScalarQpJacobian()`: Jacobian component d-`_kappa`-residual / d-`_kappa`
- `computeScalarQpOffDiagJacobian(jvar_num)`: off-diagonal Jacobian component d-`_kappa`-residual / d-`jvar`
- `computeScalarQpOffDiagJacobianScalar(svar_num)`: off-diagonal Jacobian component d-`_kappa`-residual / d-`svar`

Examples of some of these methods are shown below in [#KSB-examples]. Loops over the coupled variables wrap around these quadrature loops. The integer for the spatial variable is `jvar_num` and the integer for the
scalar variable is `svar_num`. 

Also, there are some pre-calculation routines that are called
within the quadrature loop once before the loop over spatial variable test and shape functions as well as 
before the loop over scalar components. These methods are useful for material or stabilization calculations.

- `initScalarQpResidual()`: evaluations depending on qp but independent of test functions
- `initScalarQpJacobian(jvar_num)`: evaluations depending on qp but independent of test and shape functions
- `initScalarQpOffDiagJacobian(jsvar)`: evaluations depending on qp but independent of test and shape functions

In addition to those mentioned in the [Kernel](syntax/Kernels/index.md) documentation, you have access to
several member variables inside your `KernelScalarBase` class for computing the
residual and Jacobian values in the above mentioned functions:

- `_h`, `_l`: indices for the current test and trial scalar component respectively.
- `_kappa`: value of the scalar variable this Kernel operates on; indexed by `_h` (i.e. `_kappa[_h]`).
- `_kappa_var`: ID number of this scalar variable; useful to differentiate from others.
- `_k_order`: order (number of components) of the scalar variable.

Since the test and trial "shape" functions of a scalar are "1", variables are not required for that
value. Examples of the source codes below demonstrate this fact.

!alert warning title=AD global indexing required
`ADKernelScalarBase` only works with MOOSE configured with global AD indexing (the default).

!alert note title=Parallelization of scalar contributions
While these quadrature loops are convenient for implementation in a single object, the speed of 
parallel execution may be slower due to the sequential assembly needed from each element assemblying
to the same scalar variable `_kappa`. For greater speed, the developer may instead implement the
terms for `computeScalarQpResidual()` and `computeScalarQpJacobian()` through a derived class of `ElementIntegralUserObject` as discussed at [ScalarKernels/index.md#couple-spatial].

## Examples from Source Code id=KSB-examples

As mentioned, the `computeScalarQpResidual` method +should+ be overridden for both flavors of kernels, non-AD
and AD. As an example, consider the scalar residual weak form term of the 
[`ScalarLMKernel`](/ScalarLMKernel.md) class:

\begin{equation}
  F^{(\lambda)} \equiv \int_{\Omega} \phi^h \;\text{d}\Omega - V_0 = 0 \label{eq:eq1}
\end{equation}

The `computeScalarQpResidual` method for the non-AD version of this class is
provided in [scalar-kernel-non-ad-residual], where `_value/_pp_value` is equal to $V_0$.

!listing framework/src/kernels/ScalarLMKernel.C id=scalar-kernel-non-ad-residual
         re=Real\nScalarLMKernel::computeScalarQpResidual.*?}
         caption=The C++ weak-form residual statement of [eq:eq1].

Meanwhile, the contribution to the spatial variable residual of this object is associated with [eq:eq2]
and implemented in [kernel-non-ad-residual] (note that the scalar variable `_kappa` is termed as 
$\lambda^h$ in this weak form).

\begin{equation}
  F^{(\phi)}_i \equiv \lambda^h \int_{\Omega} \varphi_i \;\text{d}\Omega \label{eq:eq2}
\end{equation}

!listing framework/src/kernels/ScalarLMKernel.C id=kernel-non-ad-residual
         re=Real\nScalarLMKernel::computeQpResidual.*?}
         caption=The C++ weak-form residual statement of [eq:eq2].

This object also overrides the `computeScalarQpOffDiagJacobian` method to define the Jacobian term related
to [eq:eq1] as shown in [non-ad-s-v-jacobian].

!listing framework/src/kernels/ScalarLMKernel.C id=non-ad-s-v-jacobian
         re=Real\nScalarLMKernel::computeScalarQpOffDiagJacobian.*?}
         caption=The C++ weak-form Jacobian for d-`_kappa`-residual / d-`jvar`.

Notice that there is a conditional to confirm that the coupled `jvar` is the focus variable `_var`, otherwise it returns zero.

Similarly, it also overrides the `computeQpOffDiagJacobianScalar` method to define the Jacobian term related
to [eq:eq2] as shown in [non-ad-v-s-jacobian].

!listing framework/src/kernels/ScalarLMKernel.C id=non-ad-v-s-jacobian
         re=Real\nScalarLMKernel::computeQpOffDiagJacobianScalar.*?}
         caption=The C++ weak-form Jacobian for d-`_var`-residual / d-`svar`.

Also notice the conditional that confirms the coupled `svar` is the focus scalar `_kappa`, otherwise it returns zero.

The AD version of this object, [`ADScalarLMKernel`](/ADScalarLMKernel.md), only requires the residual
implementation; as such it overrides `computeScalarQpResidual` and `computeQpResidual` as follows.

!listing framework/src/kernels/ADScalarLMKernel.C id=ad-s-residual
         re=ADScalarLMKernel::computeScalarQpResidual.*?}
         caption=The C++ AD weak-form residual statement of [eq:eq1].

!listing framework/src/kernels/ADScalarLMKernel.C id=kernel-ad-residual
         re=ADScalarLMKernel::computeQpResidual.*?}
         caption=The C++ AD weak-form residual statement of [eq:eq2].

Depending upon the weak form and its coupling terms between spatial and scalar variables, not all of the
methods listed in [#KSB-coupling] need to be overridden.

The scalar augmentation system is designed such that multiple scalar variables can be coupled to
an instance of the Kernel class, each focusing on one scalar from the list. This approach is similar
to how Tensor Mechanics module classes operator on one component variable of the displacement vector
field and are coupled to the other components. The developer can decide how to organize the coupling
and off-diagonal Jacobian terms in a logical way and document this for the user. Examples of two schemes
for decomposing the coupling terms and having multiple scalar variables are contained in the source files
of the Tensor Mechanics module test directory located at `modules/tensor_mechanics/test/include/kernels/`.
The input files `2dscalar.i` and `2dsole.i` are located in `modules/tensor_mechanics/test/tests/lagrangian/cartesian/total/homogenization/scalar_kernel/`. The comments within these source files should be consulted
to visualize how the rows and columns of the relevant residual and Jacobian contributions are handled.

!alert note title=Displaced mesh features untested
The displaced mesh features are not yet tested for the scalar augmentation system.

## Parameters id=KSB-parameters

There are two required parameters the user must supply for a kernel derived
from `KernelScalarBase`:

- `scalar_variable`: the focus scalar variable of the kernel, for which assembly
  of the residual and Jacobian contributions will occur. It must be a `MooseScalarVariable`.
  This parameter may be renamed in a derived class to be more physically meaningful.
- `coupled_scalar`: the name of the focus scalar variable, provided a second time
  to ensure that the dependency of the `Kernel` on this variable is detected. This
  parameter cannot be renamed.

If the `scalar_variable` parameter is not specified, then the derived class will behave
identically to a regular `Kernel`, namely without any scalar functionality. This feature
is useful if the scalar augmentation in inserted into a class structure with several
levels and not all derived members use scalar variables.

As an example, the duplicate parameter listing is shown below for the `ScalarLMKernel` object
with the `scalar_variable` parameter renamed to `kappa`:

!listing test/tests/kernels/scalar_kernel_constraint/scalar_constraint_kernel.i block=Kernels

There are also some optional parameters that can be supplied to
`KernelScalarBase` classes. They are:

- `compute_scalar_residuals`: Whether to compute scalar residuals. This
  will automatically be set to false if a `scalar_variable` parameter is not
  supplied. Other cases where the user may want to set this to false is during
  testing when the scalar variable is an `AuxVariable` and not a solution variable
  in the system.
- `compute_field_residuals`: Whether to compute residuals for the primal field
  variable. If several `KernelScalarBase` objects are used in the input file
  to compute different rows (i.e. different variables) of the global residual,
  then some objects can be targeted to field variable rows and others to scalar
  variable rows.
