!content pagination previous=tutorial01_app_development/step09_mat_props.md
                    next=tutorial01_app_development/step11_transient_kernel.md
                    margin-bottom=0px

# Step 10: Develop an AuxKernel Object

This step introduces the auxiliary system in MOOSE, which is based on the `AuxKernel` class and operates on `AuxVariables`. These two components are analogous to `Kernel` objects operating on `Variables`, e.g., a `DarcyPressure` object operating on a pressure variable. However, `AuxVariables` do not directly affect the [source/systems/NonlinearSystem.md]---the various methods that contribute to solving
[!ac](PDEs) by the Galerkin [!ac](FEM).

To demonstrate the basic use of this system, a new `VectorAuxKernel` object that computes the volumetric flux (velocity) associated with a given pressure gradient will be developed in accordance with Darcy's Law. A simple regression test for this new class will be created. It will also be implemented in the pressure vessel model of `pressure_diffusion.i`.

## Auxiliary Variables

In [Step 5](tutorial01_app_development/step05_kernel_object.md#kernels), it was mentioned that kernels compute residual contributions from the volumetric terms in a [!ac](PDE). The weak form of these terms exposes the primary dependent variable of a given problem, i.e., that which is multiplied by the test function gradient $\nabla \psi$, and which can only be determined by enforcing Dirichlet boundary conditions and solving the resulting system of equations. For example, the primary variable of Darcy's equation is pressure. In the case of solid mechanics (referred to as [Tensor Mechanics](modules/tensor_mechanics/index.md) in MOOSE), it is displacement. In MOOSE lingo, these types of variables are referred to as *nonlinear variables*.

In contrast to nonlinear variables, *auxiliary variables* are never the primary dependent. These variables can always be computed explicitly based on given information during any part of a solve procedure. But, similar to nonlinear variables, they are continuous and possibly differentiable over individual finite elements and vary based on an assumed shape function. Often, they depend on nonlinear variables and/or vice versa. For example, one auxiliary variable in Darcy's equation is the volumetric flux, which depends on the gradient of a scalar pressure field. In the case of solid mechanics, stress and strain, which both depend on the gradient of a displacement field, are considered as auxiliary variables. In structural dynamics, acceleration and velocity are considered to be auxiliary, but the displacement at any given moment depends on their values. In other cases still, auxiliary variables may have no relation to the nonlinear ones.

Auxiliary variables are declared in the [`[AuxVariables]`](syntax/AuxVariables/index.md) block and have access to the same parameters that their nonlinear counterparts do, e.g., `"order"` and `"family"`. They are designated as one of the following two types depending on their input for the `"family"` parameter:

- Elemental (`family = MONOMIAL` or `family = MONOMIAL_VEC`)
- Nodal (`family = LAGRANGE` or `family = LAGRANGE_VEC`)

It is necessary to distinguish `AuxVariables` based on [!ac](FE) shape function family because their selection dictates how they are treated by `AuxKernel` objects, which are discussed in the next section.

!alert note title=Variable coupling rules
Elemental variables can couple to any other type of variable, including nonlinear variables and other auxiliary ones. Nodal variables, however, can only couple to nonlinear variables and other nodal auxiliary ones, but not elemental ones.

## Auxiliary Kernel Objects

The [syntax/AuxKernels/index.md] in MOOSE is one for computing and setting the explicitly known values of auxiliary variables. The main advantage of [`AuxKernel`](src/auxkernels/AuxKernel.C) (or `VectorAuxKernel`) objects is that the variables they operate on are treated like any other MOOSE variable and can be used to decouple systems of equations or for various postprocessing tasks. These objects have access to the following variable members:

- `_u`, `_grad_u`\\
  Value and gradient of the variable being operated on.

- `_q_point`\\
  Coordinates of the current quadrature point. This member is only valid when operating elemental variables.

- `_qp`\\
  Current quadrature point or node index.

- `_current_elem`\\
  Pointer to the current element being operated on (elemental only).

- `_current_node`\\
  Pointer to the current node being operate on (nodal only).

One major difference between `AuxKernel` objects and `Kernel` ones is that the former do not compute residuals and, therefore, do not involve test functions $\psi$. Also, +all `AuxKernel` objects must override the `computeValue()` method+ as opposed to `computeQpResidual()` (or any of the other [virtual function members](https://en.cppreference.com/w/cpp/language/virtual) discussed in the [tutorial01_app_development/step05_kernel_object.md#kernels] section).

Every `AuxKernel` object will add a parameter `"variable"` taken from the `validParams()` method of the base class. This parameter must provide a string identifying the `AuxVariable` object to write the results to. When the variable type is elemental, the `computeValue()` method averages the values over the element [!ac](QPs) weighted by their Gaussian quadratures in proportion to the total volume, area, or length of the element. When the variable type is nodal, the values are computed directly at nodes and the `_qp` member is actually a node index.

!alert note title=Only elemental variables can couple to material properties.
`AuxKernel` objects operating on elemental variables can easily couple to material properties because both are defined at the Gauss [!ac](QPs), which will be accessed by the `computeValue()` method accordingly. If the variable is nodal, however, the [!ac](QPs) cannot be accessed and so neither can the material properties.

## Demonstration id=demo

The volumetric flux $\vec{u}$ of a fluid through a porous medium is governed by Darcy's Law as it was given in [!eqref](tutorial01_app_development/problem_statement.md#darcy). Assuming a negligible gravitational field and a homogeneous medium, the law can be expressed as

!equation id=darcy
\vec{u} = \underbrace{-\dfrac{K}{\mu} \nabla p}_{\clap{Auxiliary \, Kernel}} \in \Omega

Assuming every term in [!eqref](darcy) is known, including the pressure variable $p$, it can be developed as a `VectorAuxKernel` object. This new `MooseObject` shall accept an input for the name of the nonlinear variable holding the values for $p$ and retrieve the $K$ and $\mu$ terms from the available material properties.

In a $\R^{2}$ space, such as the axisymmetric pipe model developed in `pressure_diffusion.i`, the pressure gradient has two components, e.g., $\nabla p = \begin{bmatrix} \partial_{x} p & \partial_{y} p \end{bmatrix}^{\mathrm{T}}$. According to [!eqref](darcy), the $x$-component of the volumetric flux through a distance $L$ must be

!equation id=darcy-x
u_{x} = -\dfrac{K}{\mu} \dfrac{\partial p}{\partial x}, \enspace \forall \, x \in [0, L]

If $p$ is constant along the $y$-direction (i.e., if $\partial_{y} p = 0$), then the pressure throughout the plane $\Omega$ can be described by a univariate function of $x$; independent of $y$. Assuming that $p$ takes the form of a linear Lagrange polynomial, this function is given by

!equation id=pressure-x
p(x, y) \Rarr p(x) = \begin{bmatrix} 1 - \dfrac{x}{L} & \dfrac{x}{L} \end{bmatrix} \begin{Bmatrix} p_{0} \\ p_{1} \end{Bmatrix}

where $p_{0}$ and $p_{1}$ are the pressures at the domain boundaries: $x_{0} = 0$ and $x_{1} = L$, respectively. Differentiating [!eqref](pressure-x) and substituing it in [!eqref](darcy-x) leads to the following:

!equation id=velocity-x
u_{x} = \dfrac{K}{\mu L} \begin{bmatrix} 1 & -1 \end{bmatrix} \begin{Bmatrix} p_{0} \\ p_{1} \end{Bmatrix}

Thus, for any model that satifies the foregoing assumptions, especially $\partial_{y} p = 0$ so that $p \equiv p(x)$, $u_{x}$ is a constant monomial that depends only on $p_{0}$, $p_{1}$, $K$, $\mu$, and $L$.

In the pipe model of `pressure_diffusion.i`, $p_{0} = 4000 \, \textrm{Pa}$ and $p_{1} = 0$ are the only [!ac](BCs) applied. And since there are no pressure fluxes through any of the boundaries, the model indeed satisfies $\partial_{y} p = 0$. Therefore, [!eqref](velocity-x) can be expected to hold for this problem because the `pressure` variable uses the default parameters: `order = FIRST` and `family = LAGRANGE`. To verify that a new class designed to evaluate [!eqref](darcy) is working properly, it shall be confirmed that the L2-norm of the result in all elements is the value $\lVert \vec{u} \rVert = |u_{x}| = 0.01393 \, \textrm{m/s}$ obtained by substituting $p_{0}$, $p_{1}$, $K = 0.8451 \times 10^{-9} \, \textrm{m}^{2}$, $\mu_{f} = 7.98 \times 10^{-4} \, \textrm{Pa} \cdot \textrm{s}$, and $L = 0.304 \, \textrm{m}$. In addition, a simpler model that is also expected to satisfy [!eqref](velocity-x) shall be developed as a regression test.

### Source Code id=source-demo

To evaluate [!eqref](darcy), a new `VectorAuxKernel` object can be created and it shall be called `DarcyVelocity`. (The word "velocity" is used since $\vec{u}$ has units of distance-per-time, although it does not actually represent the flow velocity of a fluid; rather, it represents discharge-per-area). Start by making the directories to store files for objects that are part of the AuxKernels System:

!include commands/mkdir.md
         replace=['<d>', 'include/auxkernels src/auxkernels']

In `include/auxkernels`, create a file name `DarcyVelocity.h` and add the code given in [dv-header]. Here, the [`AuxKernel.h`](include/auxkernels/AuxKernel.h) header file was included because it provides the `VectorAuxKernel` base class that needs to be inherited. The `validParams()` and constructor methods were declared as `public` members. In the `protected` member declarations, the `computeValue()` method from the base class was overriden. This method returns a `RealVectorValue` data type---a tuple of $n$ values corresponding to the components of a vector in $\R^{n}$. A reference to a variable `_grad_p` was declared and will be used to couple to the pressure $p$ (set by a `DarcyPressure` object) and compute $\nabla p$. The `VariableGradient` type will numerically differentiate the input over the field before writing its point values to the `_qp` indices of `_grad_p`. Finally, two `ADMaterialProperty<Real>` variables, `_permeability` and `_viscosity`, were declared and will be used to consume the values for $K$ and $\mu$ (set by a `PackedColumn` object), which is exactly what the `DarcyPressure` class was made to do in the [previous step](tutorial01_app_development/step09_mat_props.md#source-demo).

!listing tutorials/tutorial01_app_development/step10_auxkernels/include/auxkernels/DarcyVelocity.h
         link=False
         id=dv-header
         caption=Header file for the `DarcyVelocity` class.

In `src/auxkernels`, create a file name `DarcyVelocity.C` and add the code given in [dv-source]. For the dependencies, the header file for the [`MetaPhysicL` namespace](https://github.com/roystgnr/MetaPhysicL/blob/master/src/numerics/include/metaphysicl/raw_type.h) was included in addition to `DarcyVelocity.h`, because it provides a simple method for accessing the raw values of [!ac](AD) types, which are usually a complex set of containers storing a variable and all of its derivatives. The method will be used to handle the `ADMaterialProperty` data.

The only input that needs to be appended to those defined by `VectorAuxKernel::validParams()` is the name of the nonlinear variable containing the values for $p$. Thus, the `addRequiredCoupledVar()` method was used to define a parameter `"pressure"`. This `InputParameters` member is like `addRequiredParam()`, but the only type of data it accepts is a string identifying the variable object, for which, if not found, will cause an error.

In the constructor definition, all $n$ components of the gradient of the variable provided to the `"pressure"` parameter were retrieved by the `coupledGradient()` method from the [`Couplable`](src/interfaces/Coupleable.C) class---a member of the [framework_development/interfaces/index.md] System in MOOSE. The data returned is then used to set `_grad_p`. Next, two material properties by the names `"permeability"` and `"viscosity"` are retrieved from the available material properties and used to set `_permeability` and `_viscosity`, respectively. Finally, the required `computeValue()` method is defined with the right-hand side of [!eqref](darcy) discretized over all [!ac](QPs). Here, the derivatives of the `_permeability` and `_viscosity` variables are not of concern (and are zero at all [!ac](QPs) anyways), but the `MetaPhysicL::raw_value()` function will ensure that the value $K / \mu$ is returned when the [!ac](AD) data is passed to it.

!listing tutorials/tutorial01_app_development/step10_auxkernels/src/auxkernels/DarcyVelocity.C
         link=False
         id=dv-source
         caption=Source file for the `DarcyVelocity` class.

Be sure to recompile the application before proceeding:

!include commands/make.md

### Input File id=input-demo

With the new `DarcyVelocity` class available, it is now possible to compute the volumetric flux $\vec{u}$ of the fluid in the pressure vessel model. First, recall that earlier in the [#demo] section it was noted that $u_{x}$ is given by a constant monomial expression, i.e., [!eqref](velocity-x). It could also be shown that $\vec{u}$ is =a vector of constant monomial expressions= when $p(x, y)$ is approximated by a linear Lagrange polynomial, even if $\partial_{y} p \; {=}\mathllap{\small{/}\,} \; 0$. It therefore makes sense to add the `[AuxVariables]` block to `pressure_diffusion.i` using the following syntax:

!listing tutorials/tutorial01_app_development/step10_auxkernels/problems/pressure_diffusion.i
         block=AuxVariables
         link=False

The `[velocity]` block creates an elemental variable with `order = CONSTANT` and `family = MONOMIAL_VEC`, where `MONOMIAL_VEC` types are just a vector of `MONOMIAL` ones. This is good, because that's exactly what $\vec{u}$ should be and it is a requirement that variables associated with `DarcyVelocity` objects be `RealVectorValue` objects (or something similar).

Proceed with adding the following `[AuxKernels]` block to `pressure_diffusion.i`:

!listing tutorials/tutorial01_app_development/step10_auxkernels/problems/pressure_diffusion.i
         block=AuxKernels
         link=False

Here, a `DarcyVelocity` object was created. The associated variable that stores the results was identified along with the nonlinear variable associated with the `DarcyPressure` object. The `"execute_on"` parameter is made available through the base class and is used to control when the object runs via the [`ExecFlagEnum`](src/utils/ExecFlagEnum.C) methods. For this demonstration, the velocity is calculated based on the current steady-state solution. Setting `execute_on = TIMESTEP_END` ensures that the `DarcyVelocity` object is constructed only after a `DarcyPressure` object has computed the current solution $p$.

Now, execute the application:

!listing language=bash
cd ~/projects/babbler/problems
../babbler-opt -i pressure_diffusion.i

If the program ran successfully, a formal test of the `DarcyVelocity` class is in order. Start by creating a directory to store the test files:

!include commands/mkdir.md
         replace=['<d>', 'test/tests/auxkernels/darcy_velocity']

In this folder, create a file named `darcy_velocity_test.i` and add the inputs given in [dv-test]. The model created in this file is the same as in `darcy_pressure_test.i`, except with `[AuxVariables]` and `[AuxKernels]` blocks added using the same syntax just demonstrated for `pressure_diffusion.i`.

Notice in [dv-test] that the nonlinear variable `pressure` is created with `order = FIRST` and `family = LAGRANGE`, even though they are the defaults. This is to emphasize a major assumption behind the test, as it is a requirement if the resulting velocity vector is to be composed of constant monomials. Also notice that `l_tol = 1e-07` is set in the `[Executioner]` block. This forces the algebraic solvers to converge on an absolute residual that is at least $10^{-7}$ less than the initial residual before the `PJFNK` type solver attempts to compute a new Jacobian or, rather, its action on the solution vector (see [source/systems/NonlinearSystem.md#jacobian_definition]). Using a tighter relative linear tolerance improves the overall accuracy of the solution. For testing purposes, it needs to be verified that the new `DarcyVelocity` class produces the expected result $\lVert \vec{u} \rVert = |u_{x}|$. However, it was found that the default linear tolerance of $10^{-5}$ yields very slight deviations in $u_{x}$ between elements.

!listing tutorials/tutorial01_app_development/step10_auxkernels/test/tests/auxkernels/darcy_velocity/darcy_velocity_test.i
         link=False
         id=dv-test
         caption=Input file to test the `DarcyVelocity` class with an `Exodiff` object.

Run the input file and confirm that the linear solver always iterates until it computes an absolute residual that is at least $10^{-7}$ less than the one computed on the zero-th iteration:

!listing language=bash
cd ~/projects/babbler/test/tests/auxkernels/darcy_velocity
../../../../babbler-opt -i darcy_velocity_test.i

### Results id=result-demo

Use PEACOCK to visualize the solution of the `pressure_diffusion.i` model:

!include commands/peacock_r.md
         replace=['<d>', 'problems',
                  '<e>', 'pressure_diffusion_out']

Use the dropdown menu to select the "velocity_" variable and render its contours. Also check that the selected component in the adjacent dropdown is "Magnitude" and confirm that the result resembles the one shown in [results-1].

!media tutorial01_app_development/step10_result01.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=results-1
       caption=Pressure vessel model results for $\lVert \vec{u} \rVert$ indicating differences smaller than $10^{-5}$ between any two elements.

From [results-1], it can be concluded that the `DarcyVelocity` object produced the expected results of $\lVert \vec{u} \rVert = 0.01393 \, \textrm{m/s}$ at all points in the mesh. Still, the renderer seems to be registering slightly different values between elements as indicated by the many different colors displayed. For practical purposes, these differences are negligible, but they can be neutralized by using tighter solver tolerances as discussed in the [#input-demo] section. To test this hypothesis, there's no need to modify the input file as any parameter can be overridden from the terminal, e.g., the following will rerun it with a relative linear tolerance of $10^{-16}$:

!listing language=bash
cd ~/projects/babbler/problems
../babbler-opt -i pressure_diffusion.i Executioner/l_tol=1e-16

Next, rerun `peacock -r` on the new results and observe the uniformity in the contours for $\lVert \vec{u} \rVert$, as illustrated in [results-2]. Note that the contour mappings were rounded off to 15 decimal digits. The defaults for the "Min" and "Max" fields in the ExodusViewer tab of PEACOCK might have to be trimmed to this many significant digits to achieve a solid contour like the one shown.

!media tutorial01_app_development/step10_result02.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=results-2
       caption=Pressure vessel model results for $\lVert \vec{u} \rVert$ using a relative linear tolerance of $10^{-16}$. Every element shows exactly the predicted velocity magnitude of $0.01393 \, \textrm{m/s}$.

!alert! tip title=Be aware of the many different types of controllable tolerances.
There is usually a variety of parameters available to `Executioner` objects that are used to set solver tolerances. Sometimes, even slight changes to them can have profound numerical effects, and certain types of tolerances supersede others. For example, the same desired effect of constantness (up to 16 decimal digits---the available precision for [IEEE 754 decimal64](https://en.wikipedia.org/wiki/Decimal64_floating-point_format) formatted floats (referred to as *double precision* in C++)) can be achieved by setting `nl_rel_tol = 1e-12`, but more nonlinear iterations are required in this case.

*For more information about common solver parameters, please see the [source/executioners/Steady.md] or [source/executioners/Transient.md] page.*
!alert-end!

Now, use PEACOCK to inspect the results of the `DarcyVelocity` test:

!include commands/peacock_r.md
         replace=['<d>', 'test/tests/auxkernels/darcy_velocity',
                  '<e>', 'darcy_velocity_test_out']

Again, select the contours for the velocity magnitude and confirm that a solid contour is rendered up to 15 significant digits, as illustrated in [results-3]. This model was created by [dv-test] and uses $p_{0} = 0$, $p_{1} = 1 \, \textrm{Pa}$, and $L = 1 \, \textrm{m}$. And since the `PackedColumn` object used the defaults for the `"diameter"` and `"viscosity"` parameters, $K = 0.8451 \times 10^{-9} \, \textrm{m}^{2}$ and $\mu = 7.98 \times 10^{-4} \, \textrm{Pa} \cdot \textrm{s}$. Substituting these values in [!eqref](velocity-x) yields

!equation
u_{x} = \dfrac{0.8451 \times 10^{-9}}{7.98 \times 10^{-4}} \begin{bmatrix} 1 & -1 \end{bmatrix} \begin{Bmatrix} 0 \\ 1 \end{Bmatrix} = -1.059 \times 10^{-6} \, \textrm{m/s}

Thus, the figure indicates that the `darcy_velocity_test.i` model must be correct since $|u_{x}| = 1.059 \times 10^{-6}$.

!media tutorial01_app_development/step10_result03.png
       style=width:64%;margin-left:auto;margin-right:auto;
       id=results-3
       caption=Rendering of the results for $\lVert \vec{u} \rVert$ produced by the `DarcyVelocity` test. Every element shows exactly the predicted velocity magnitude of $1.059 \times 10^{-6} \, \textrm{m/s}$.

### Test id=test-demo

Since the results of the `darcy_velocity_test.i` input file have been deemed good, the ExodusII output can now become a certified gold file:

!include commands/new_gold.md
         replace=['<d>', 'auxkernels/darcy_velocity',
                  '<e>', 'darcy_velocity_test_out']

!alert note icon-name=create prefix=False title=TASK:$\;\;$Create the specification for the `DarcyVelocity` test.
With the `darcy_velocity_test.i` file written and the `darcy_velocity_test_out.e` file moved to `gold`, a test specification file named `tests` can be generated in `test/tests/auxkernels/darcy_velocity` and is left up to the reader to determine the syntax. If necessary, please review [Step 8](tutorial01_app_development/step08_test_harness.md).

Be sure to write the `tests` file as instructed above. Finally, run `TestHarness`:

!include commands/run_tests.md

If the tests passed, the terminal output should look something like that shown below.

```
test:kernels/darcy_pressure.test .......................................................................... OK
test:materials/packed_column.test ......................................................................... OK
test:kernels/simple_diffusion.test ........................................................................ OK
test:auxkernels/darcy_velocity.test ....................................................................... OK
--------------------------------------------------------------------------------------------------------------
Ran 4 tests in 0.4 seconds. Average test time 0.1 seconds, maximum test time 0.1 seconds.
4 passed, 0 skipped, 0 pending, 0 failed
```

### Commit id=commit-demo

Add all of the changes made in this step to the git tracker:

!include commands/git_add.md

!alert tip title=Keep tabs on local changes.
As changes to the repository are made, it can be helpful to frequently check the outputs from `git status` and/or `git diff`. Once a file is in good shape, it can be staged using a `git add` and further changes can be made before committing, but be careful when using wildcards!

Now, commit and push the changes to the remote repository:

!include commands/git_commit.md
         replace=['<m>', '"developed auxkernel for computing the velocity associated with a pressure gradient in accordance with Darcy\'s law"']

!content pagination previous=tutorial01_app_development/step09_mat_props.md
                    next=tutorial01_app_development/step11_transient_kernel.md
