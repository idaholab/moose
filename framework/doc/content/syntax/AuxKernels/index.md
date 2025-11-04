# AuxKernels System

The AuxKernel system mimics the [syntax/Kernels/index.md] but compute values that can be defined
explicitly with a known function. There are two main use cases for the AuxKernel system: computing
a quantity that varies with space and time for postprocessing purposes or for decoupling systems
of equations. Examples for both of these use cases shall be discussed further in the following
sections.

Creating a custom AuxKernel object is done by creating a new C++ object that inherits from
`AuxKernel`, `VectorAuxKernel` or `ArrayAuxKernel` and overriding the `computeValue` method,
which returns a scalar (`Real`), vector (`RealVectorValue`) or a Eigen vector (`RealEigenVector`)
for the three types respectively. A forth type (`AuxScalarKernel`) also exists, but the syntax for
these objects is different and detailed in the [syntax/AuxScalarKernels/index.md].

AuxKernel objects, like Kernel objects, must operate on a variable. Thus, there is a required
parameter ("variable") that indicates the variable that the AuxKernel object is computing. These
variables are defined in the [AuxVariables](syntax/AuxVariables/index.md) block of the input file.
AuxKernel objects derived from `AuxKernel`, `VectorAuxKernel` or `ArrayAuxKernel` operate on
standard scalar, vector or array field variables respectively. For example the following input
file snippet creates an auxiliary variable suitable for use with an `VectorAuxKernel`.

!listing vector_function_aux.i block=AuxVariables

## Nodal vs Elemental AuxKernel Objects

There are two flavors of AuxKernel objects: nodal and elemental. The distinction is based on the
type of variable that is being operated on by the object. If the variable family is `LAGRANGE` or
`LAGRANGE_VEC` then the AuxKernel will behave as nodal. If the variable family is `MONOMIAL` then
the AuxKernel will behave as elemental.

The difference is based on how the `computeValue` method of the object is called when the kernel
is executed. In the nodal case the `computeValue` method will be executed on each +node+ within the
finite element mesh and the value returned from the method will directly assign the value
of the shape function at that node.

In the elemental case the `computeValue` method will be executed on each quadrature point of an
+element+ of the finite element mesh. The values computed at the quadrature points are used to
perform the correct finite element projection automatically and set the values for the degrees
of freedom. This is achieved by assembling and solving a *local* finite element projection
problem for each and every element in the domain (or blocks) of interest.
Typically, in the elemental case the order of the monomial finite element is set to
constant so there is a single DOF per element, but higher monomials are also supported.

As is evident by the functionality detailed, the distinction between the two arises from the nature
of the finite element shape functions. For Lagrange shape functions the DOF values correspond with
the nodes, while for elemental shape functions the DOF values are not associated with nodes.

The same AuxKernel object can be designed work both as elemental or nodal, for example the
`computeValue` method for the [FunctionAux.md] object properly handles using the correct spatial
location based on if the object is nodal or elemental with the `isNodal` method.

## Block vs Boundary Restricted AuxKernel Objects

While auxiliary variables are always defined on mesh subdomains, MOOSE allows auxiliary kernels to be either block (mesh subdomain) or boundary restricted.
When an auxiliary kernel is boundary restricted, it evaluates an auxiliary variable only on the designated boundaries.
Because of this, the auxiliary variable will only have meaningful values on the boundaries even though it is defined on mesh subdomains.
When an auxiliary kernel is block restricted, the variable that it evaluates must be defined on a subdomain covering the blocks where the auxiliary kernel is defined.
When an auxiliary kernel is boundary restricted, the variable must be defined on a subdomain that all the sides on the boundaries are connected with.
An elemental auxiliary variable defined on an element that has multiple boundary sides cannot be properly evaluated within a boundary restricted auxiliary kernel because elemental auxiliary variables can only store one value per element.
Users can split the boundaries and define multiple elemental auxiliary variables for each split to avoid the situation of element connecting with multiple boundary sides.

!listing auxkernels/FunctionAux.C re=Real\sFunctionAux::compute.*}

Nodal AuxKernel objects abuse the notion of quadrature points, the `_qp` member variable is set
to zero, but still must be used to access coupled variable values and material properties. This
is done to allow the syntax to be consistent regardless of the AuxKernel flavor: nodal or elemental.

## Mortar Nodal Auxiliary Kernel Objects

In order to compute properties in the mortar sense, it is necessary to loop over the mortar segment
mesh to spatially integrate variables. `MortarNodalAuxKernel`s offer this functionality where these "weighted" variables,
which intervene in the computation of contact constraints and their residuals, can be coupled to generate the desired output value.
Therefore, if postprocessing of mortar quantities is required, nodal mortar auxiliary kernels can be employed.
Objects inheriting from `MortarNodalAuxKernel` allow for said operations on the mortar lower-dimensional domains featuring similar
functionality to other nodal auxiliary kernels, including the possibility of computing quantities in an
`incremental` manner.

## Execute Flags

AuxKernel objects inherit from the [SetupInterface.md] so they include the "execute_on" variable.
By default this parameter is set to `EXEC_LINEAR` and `EXEC_TIMESTEP_END`. The `EXEC_LINEAR` flag
is set because it is possible to couple values compute by an AuxKernel object to other objects
such as Kernel or Material objects that are used in the residual calculation. In order to ensure
that the values from the auxiliary variable are correct during the iterative solve they are computed
for each iteration.

However, if the auxiliary variable be computed is not being coupled to objects computing the
residual evaluating the AuxKernel on each linear iteration is not necessary and can slow down the
execution of a simulation. In this case, the `EXEC_LINEAR` flag should be removed. Likely the
`EXEC_INITIAL` flag should be added to perform the auxiliary variable calculation during the initial
setup phase as well.

## Populating lower-dimensional auxiliary variables

Lower-dimensional auxiliary variables may be populated using boundary restricted
auxiliary kernels. The boundary restriction of the aux kernel should be
coincident with (a subset of) the lower-dimensional blocks that the
lower-dimensional variable lives on. Using a boundary restricted auxiliary
kernel as opposed to a lower-d block-restricted auxiliary kernel allows pulling
in coincident face evaluations of higher-dimensional variables and material
properties as well as evaluations of coupled lower-dimensional variables.

## Example A: Post processing with AuxKernel

The following example is extracted from step 4 of the
[Darcy Flow and Thermomechanics Tutorial](darcy_thermo_mech/index.md optional=True). Consider Darcy's
Law for flow in porous media neglecting changes in time and gravity:

\begin{equation}
\label{darcy}
-\nabla\cdot\frac{\mathbf{K}}{\mu}\nabla p = 0,
\end{equation}
where $\mathbf{K}$ is the permeability tensor, $\mu$ is the fluid viscosity, and $p$ is the
pressure and the velocity ($\vec{u}$) may be computed as:

\begin{equation}
\label{darcy_vel}
\vec{u} = \frac{\mathbf{K}}{\mu}\nabla p.
\end{equation}

The left-hand side of [darcy] would be solved with a nonlinear variable and an appropriate
[Kernel object](syntax/Kernels/index.md). The AuxKernel system can be used computing the velocity
following [darcy_vel]. In the tutorial the exact calculation is performed using the DarcyVelocity
object, the header and source files for this object are listed below.

!listing step04_velocity_aux/include/auxkernels/DarcyVelocity.h

!listing step04_velocity_aux/src/auxkernels/DarcyVelocity.C


## Example B: Decoupling Equations

Auxiliary variables may be used interchangeably with nonlinear variables with respect to coupling
allowing complicated systems of equations to be decoupled for solving individually. This is very
useful for testing and validation.

Consider the heat equation with an advective term that is coupled to the pressure computed
in [darcy] as in step 6 of the
[Darcy Flow and Thermomechanics Tutorial](darcy_thermo_mech/index.md optional=True):

\begin{equation}
\label{heat}
C\left( \frac{\partial T}{\partial t} + \epsilon \vec{u}\cdot\nabla T \right) - \nabla \cdot k \nabla T = 0,
\end{equation}
where $T$ is temperature, $C$ is the heat capacity, $k$ is the thermal conductivity, and
$\epsilon$ is the porosity. The advective term ($\epsilon\vec{u}\cdot\nabla T$) is computed in a
kernel object ([step06_coupled_darcy_heat_conduction/src/kernels/DarcyAdvection.C]) and requires
the pressure variable be provided as a variable:

!listing step06_coupled_darcy_heat_conduction/src/kernels/DarcyAdvection.C line=addRequiredCoupledVar

For testing purposes is it not desirable to include the solve for the pressure variable when
examining the correctness of the heat equation solve, so an auxiliary variable that is assigned an
arbitrary function of space and time is used instead. The following input file snippet demonstrates
the decoupling of the pressure variable by computing it using an AuxVariable the FunctionAux object.

!listing step6c_decoupled.i block=AuxVariables AuxKernels


!syntax list /AuxKernels objects=True actions=False subsystems=False

!syntax list /AuxKernels objects=False actions=False subsystems=True

!syntax list /AuxKernels objects=False actions=True subsystems=False
