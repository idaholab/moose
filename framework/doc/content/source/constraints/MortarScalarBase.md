# MortarScalarBase

The `MortarScalarBase` is part of the scalar augmentation system to complement the [ScalarKernel](syntax/ScalarKernels/index.md) class. Its principal purpose is to add standard
quadrature loops and assembly routines to handle the contributions from a single added
scalar variable to a [Mortar Constraint](syntax/Constraints/index.md) class, including the entire row
of the Jacobian.
These routines will assist with representing weak form terms in a partial differential equation involving
scalar variables integrated over the surface of a non-conforming interface (mortar surface). As usual,
this piece of physics is referred to
as the "residual" and is evaluated at integration quadrature points along that interface.
To implement your own physics in MOOSE, you create your own mortar constraint by subclassing the MOOSE `MortarScalarBase` class.

The mortar scalar augmentation system supports the use of [!ac](AD) for residual calculations, as such
there are two options for creating field-scalar coupling objects:
`MortarScalarBase` and `ADMortarScalarBase`. To further understand
automatic differentiation, please refer to the [automatic_differentiation/index.md] page for more
information.

Developers should read all sections; users can find [#MSB-parameters] described at the bottom.

## Creation of Mortar Constraint Scalar Coupling Classes id=MSB-coupling

`MortarConstraint` objects are designed to handle weak-form contributions from both surface-based
lower-dimensional (Lagrange multiplier) variables and domain-based (primary and secondary) spatial
variables. For these three +focus+ spatial variables, its job is to contribute to the associated rows
of the residual and the Jacobian matrix. Herein, as in the source code of `MortarConstraints`, these
spatial variables will be called (see Mortar [syntax/Constraints/index.md#MC-parameters] for the analogous
input parameter names):

- `_var`: Corresponds to a Lagrange Multiplier variable that lives on the
  lower dimensional block on the secondary face
- `_secondary_var`: Primal variable on the secondary side of the mortar interface
  (lives on the interior elements)
- `_primary_var`: Primal variable on the primary side of the mortar interface
  (lives on the interior elements).

The three focus spatial variables `_var`, `_secondary_var`, and `_primary_var` are each indicated by the
name `var` below, and are differentiated by the `mortar_type` flag as one of "Primary". "Secondary", or
"Lower". In a coupled (multi-phyics) weak form, all interface integral terms containing the test function
of one of these three variables are potential candidates for `MortarConstraint` contributions.

The philosphy of the scalar augmentation class `MortarScalarBase` is to add a single focus scalar variable
referred to as `_kappa` to the `MortarConstraint` object so that all terms in the coupled weak form that
involve the spatial variables, scalar variable, and/or their test functions can be assembled in one
or multiple class instances.
This philosophy is similar to how the lower dimensional variable `_lambda` is added to the element faces
of `DGKernel` and `IntegratedBC` objects associated with the hybrid finite element method (HFEM). Documentation for that approach can be found [HFEMDiffusion](source/dgkernels/HFEMDiffusion.md) and [HFEMDirichletBC](source/bcs/HFEMDirichletBC.md) along with the base classes `DGLowerDKernel` and `LowerDIntegratedBC`.

In a `MortarScalarBase` subclass, a naming scheme is established for the quadrature point methods of the two
variable types: methods contributing to the test function of `_kappa` have "Scalar" near the front
and methods contributing to the trial function of scalar variables in the Jacobian have "Scalar"
at the end. The `computeScalarQpResidual()` function +should+ be overridden (see [#MSB-parameters]
for cases where the scalar should be suppressed). The `computeQpResidual(mortar_type)` function +must+ be
overridden as usual for `MortarConstraint`, although it may return zero.

For non-AD objects, several contributions to the Jacobian matrix can be optionally overridden for use in
Newton-type nonlinear solvers. As mentioned later, the developer should choose and document which terms
(rows) of the residual and terms (rows and columns) of the Jacobian will be attributed to an instance of
the developed class. These choices can be motivated by whether some terms in the weak form can be or have
already been implemented within other MOOSE classes.

- `computeQpJacobian(jacobian_type, jvar_num)`: Jacobian component d-`var`-residual / d-`jvar`
- `computeQpOffDiagJacobianScalar(mortar_type, svar_num)`: Jacobian component d-`var`-residual / d-`svar`
- `computeScalarQpJacobian()`: Jacobian component d-`_kappa`-residual / d-`_kappa`
- `computeScalarQpOffDiagJacobian(mortar_type, jvar_num)`: Jacobian component d-`_kappa`-residual / d-`jvar`
- `computeScalarQpOffDiagJacobianScalar(svar_num)`: Jacobian component d-`_kappa`-residual / d-`svar`

Similar to the `mortar_type` flag
mentioned above, the `jacobian_type` flag is used to distinguish the couplings between the test and trial
functions of the three focus spatial variables. For example, `jacobian_type = LowerSecondary` indicates
that the linearized weak form term from the `_var` test function and `_secondary_var` trial function should
be evaluated. All nine combinations from the three focus variables are visited during the loops that call
`computeQpJacobian(jacobian_type, jvar_num)`. Also, loops over the coupled variables wrap around these quadrature loops: the integer for the spatial variable is `jvar_num` and the integer for the
scalar variable is `svar_num`. Note that the `jvar_num` integer is different for `_var` and `_secondary_var`
and for any other spatial variables coupled by the developer through derived classes of `MortarScalarBase`,
so consult the examples of these methods below in [#MSB-examples] for how to query the current target
using logical tests.

Also, there are some pre-calculation routines that are called
within the quadrature loop once before the loop over spatial variable test and shape functions as well as 
before the loop over scalar components. These methods are useful for material or stabilization calculations.

- `initScalarQpResidual()`: evaluations depending on qp but independent of test functions
- `initScalarQpJacobian(jvar_num)`: evaluations depending on qp but independent of test and shape functions
- `initScalarQpOffDiagJacobian(jsvar)`: evaluations depending on qp but independent of test and shape functions

In addition to those mentioned in the [MortarConstraints](syntax/Constraints/index.md) documentation,
you have access to several member variables inside your `MortarScalarBase` class for computing the
residual and Jacobian values in the above mentioned functions:

- `_h`, `_l`: indices for the current test and trial scalar component respectively.
- `_kappa`: value of the scalar variable this `MortarConstraint` operates on; indexed by `_h` (i.e. `_kappa[_h]`).
- `_kappa_var`: ID number of this scalar variable; useful to differentiate from others.
- `_k_order`: order (number of components) of the scalar variable.

Since the test and trial "shape" functions of a scalar are "1", variables are not required for that
value. Examples of the source codes below demonstrate this fact.

!alert warning title=AD global indexing required
`ADMortarScalarBase` only works with MOOSE configured with global AD indexing (the default).

## Examples from Source Code id=MSB-examples

As mentioned, the `computeScalarQpResidual` method +should+ be overridden for both flavors of 
mortar constraints, non-AD and AD. As an example, consider the scalar residual weak form terms of the 
[`PeriodicSegmentalConstraint`](/PeriodicSegmentalConstraint.md) class:

\begin{equation}
  \langle \vec{\kappa} \cdot \llbracket \vec{x} \rrbracket, \lambda \rangle _{\Gamma^+} - \langle \vec{\kappa} \cdot \llbracket \vec{x} \rrbracket, \vec{\sigma} \cdot \hat{n} \rangle _{\Gamma^+} \label{eq:eq1}
\end{equation}

The `computeScalarQpResidual` method for the non-AD version of this class is
provided in [PSC-non-ad-residual], where `_kappa_aux` is equal to $\sigma$.

!listing framework/src/constraints/PeriodicSegmentalConstraint.C id=PSC-non-ad-residual
         re=Real\nPeriodicSegmentalConstraint::computeScalarQpResidual.*?}
         caption=The C++ weak-form residual statement of [eq:eq1].

Meanwhile, the contribution to the lower spatial variable residual of this object is associated with [eq:eq2]
and implemented in [lower-non-ad-residual] (note that the scalar variable `_kappa` is termed as 
$\vec{\epsilon}$ in this weak form).

\begin{equation}
    \langle \mu , \vec{\epsilon} \cdot \llbracket \vec{x} \rrbracket \rangle _{\Gamma^+} \label{eq:eq2}
\end{equation}

!listing framework/src/constraints/PeriodicSegmentalConstraint.C id=lower-non-ad-residual
         re=Real\nPeriodicSegmentalConstraint::computeQpResidual.*?}
         caption=The C++ weak-form residual statement of [eq:eq2].

The other contributions to the lower spatial variable residual of this object is associated with [eq:eq3]
and implemented in [EVC-non-ad-residual] within the [`EqualValueConstraint`](/EqualValueConstraint.md) class.

\begin{equation}
     -\langle \mu , \llbracket u \rrbracket \rangle _{\Gamma^+} \label{eq:eq3}
\end{equation}

!listing framework/src/constraints/EqualValueConstraint.C id=EVC-non-ad-residual
         re=case Moose::MortarType::Lower.*?}
         caption=The C++ weak-form residual statement of [eq:eq3].

For an example of the contributions to `_primary_var` and `_secondary_var` residuals, consider the 
penalty version of the periodic constraint associated with [eq:eq4]
and implemented in [PPSC-non-ad-residual] within the [`PenaltyPeriodicSegmentalConstraint`](/PenaltyPeriodicSegmentalConstraint.md) class.

\begin{equation}
 - \langle \llbracket w \rrbracket,\tau \vec{\epsilon} \cdot \llbracket \vec{x} \rrbracket \rangle _{\Gamma^+} \label{eq:eq4}
\end{equation}

!listing framework/src/constraints/PenaltyPeriodicSegmentalConstraint.C id=PPSC-non-ad-residual
         re=Real\nPenaltyPeriodicSegmentalConstraint::computeQpResidual.*?}
         caption=The C++ weak-form residual statement of [eq:eq4].

The `PeriodicSegmentalConstraint` class also overrides the `computeScalarQpOffDiagJacobian` method
to define the Jacobian term related to [eq:eq1] as shown in [non-ad-PSC-s-v-jacobian].

!listing framework/src/constraints/PeriodicSegmentalConstraint.C id=non-ad-PSC-s-v-jacobian
         re=Real\nPeriodicSegmentalConstraint::computeScalarQpOffDiagJacobian.*?}
         caption=The C++ weak-form Jacobian for d-`_kappa`-residual / d-`jvar`.

Notice that there is a conditional to confirm that the coupled `jvar` is the focus lower variable `_var`, otherwise it returns zero.

Similarly, it also overrides the `computeQpOffDiagJacobianScalar` method to define the Jacobian term related
to [eq:eq2] as shown in [non-ad-PSC-v-s-jacobian].

!listing framework/src/constraints/PeriodicSegmentalConstraint.C id=non-ad-PSC-v-s-jacobian
         re=Real\nPeriodicSegmentalConstraint::computeQpOffDiagJacobianScalar.*?}
         caption=The C++ weak-form Jacobian for d-`_var`-residual / d-`svar`.

Also notice the conditional that confirms the coupled `svar` is the focus scalar `_kappa`, otherwise it returns zero.

The AD version of this object, [`ADPeriodicSegmentalConstraint`](/ADPeriodicSegmentalConstraint.md),
only requires the residual implementation; as such it overrides `computeScalarQpResidual`
and `computeQpResidual` as follows.

!listing framework/src/constraints/ADPeriodicSegmentalConstraint.C id=PSC-ad-s-residual
         re=ADPeriodicSegmentalConstraint::computeScalarQpResidual.*?}
         caption=The C++ AD weak-form residual statement of [eq:eq1].

!listing framework/src/constraints/ADPeriodicSegmentalConstraint.C id=PSC-ad-residual
         re=ADPeriodicSegmentalConstraint::computeQpResidual.*?}
         caption=The C++ AD weak-form residual statement of [eq:eq2].

Depending upon the weak form and its coupling terms between spatial and scalar variables, not all of the
methods listed in [#MSB-coupling] need to be overridden.

The scalar augmentation system is designed such that multiple scalar variables can be coupled to
an instance of the `MortarConstraint` class, each focusing on one scalar from the list. This approach is
similar to how Tensor Mechanics module classes operator on one component variable of the displacement vector
field and are coupled to the other components. The developer can decide how to organize the coupling
and off-diagonal Jacobian terms in a logical way and document this for the user.

An example for decomposing the coupling terms and having multiple scalar variables are
contained in the source files of the framework test directory as well as input file
`testperiodicsole.i`, with listings below. The comments within these header and source files
should be consulted to visualize how the rows and columns of the relevant residual and Jacobian
contributions are handled.

!listing test/include/constraints/TestPeriodicSole.h id=TPS-residual
         re=/// Test object to illustrate.*?}
         caption=Decomposition of spatial and scalar variable contributions by x and y components.

!listing test/tests/mortar/periodic_segmental_constraint/testperiodicsole.i block=Constraints

!alert note title=Displaced mesh features untested
The displaced mesh features are not yet tested for the scalar augmentation system.

## Parameters id=MSB-parameters

There is one required parameters the user must supply for a mortar constraint derived
from `MortarScalarBase`:

- `scalar_variable`: the focus scalar variable of the mortar constraint, for which assembly
  of the residual and Jacobian contributions will occur. It must be a `MooseScalarVariable`.
  This parameter may be renamed in a derived class to be more physically meaningful.

If the `scalar_variable` parameter is not specified, then the derived class will behave
identically to a regular `MortarConstraint`, namely without any scalar functionality. This feature
is useful if the scalar augmentation in inserted into a class structure with several
levels and not all derived members use scalar variables.

As an example, the parameter listing is shown below for the `PeriodicSegmentalConstraint` object
with the `scalar_variable` parameter renamed to `epsilon`:

!listing test/tests/mortar/periodic_segmental_constraint/periodic_simple2d.i block=Constraints

Note: to avoid an error message "Variable 'kappa' does not exist in this system", the following
block should be added to the input file:

!listing test/tests/mortar/periodic_segmental_constraint/periodic_simple2d.i block=Problem

There is one optional parameters that can be supplied to`MortarScalarBase` classes:

- `compute_scalar_residuals`: Whether to compute scalar residuals. This
  will automatically be set to false if a `scalar_variable` parameter is not
  supplied. Other cases where the user may want to set this to false is during
  testing when the scalar variable is an `AuxVariable` and not a solution variable
  in the system. This parameter has a similar usage as the `compute_lm_residuals`
  and `compute_primal_residuals` for all Mortar objects.
