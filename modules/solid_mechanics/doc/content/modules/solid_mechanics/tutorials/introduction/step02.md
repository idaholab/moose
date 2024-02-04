# Step 2 - Adding boundary conditions

In the previous step we set up a basic small strain mechanics simulation that
did... nothing. In this step we're adding a load to the top and we'll fix the
displacements on the bottom surface of our block.

!listing modules/tensor_mechanics/tutorials/introduction/mech_step02.i

## Input file

### `BCs`

BCs stands for *boundary conditions*. Those apply to the boundaries (or
sidesets) of the simulation domain. In all boundary condition objects you will
see the mandatory `boundary` parameter, which expects a list of sideset names or
IDs.

#### `DirichletBC`

The two [`DirichletBC`](DirichletBC.md) boundary conditions are both set on the
*bottom* surface of the simulation domain. This fixes the `disp_x` and `disp_y`
variables to 0 respectively. Check the
[list of available boundary conditions](tutorials/introduction/supplemental02.md).

#### `Pressure`

You may have noticed `[Pressure]` block looking different than the other two
boundary conditions. This is because that block is a custom
[action](PressureAction.md) syntax. Instead of a `variable` parameter it uses
the `displacements` parameter defined in the global parameters block above. The
action sets up a [`Pressure`](Pressure.md) boundary condition for each
displacement variable. We will see other examples of *actions* later on.

Using the [!param](/BCs/Pressure/PressureAction/function) parameter we supply a
time dependent applied pressure. We are taking advantage of a MOOSE shorthand
again here. Any time a `FunctionName` type parameter is requested the user can
instead supply a [parsed function expression](MooseParsedFunction.md) directly.
If you need to specify a different type of [function](Functions/index.md) or
need to reuse a single function multiple times in the input file, you should
explicitly add a function object under the `[Functions]` top level block. This
pressure action could have instead been written as

```
[Functions]
  [applied_pressure_function]
    type = ParsedFunction
    value = 1e7*t
  []
[]

[BCs]
  [Pressure]
    [top]
      boundary = top
      function = applied_pressure_function
    []
  []
[]

```

### `Preconditioning`

The [`[Preconditioning]`](syntax/Preconditioning/index.md) block lets the user
specify the which parts of the Jacobian matrix are built. Here we're selecting
the [single matrix preconditioner](SingleMatrixPreconditioner.md) with the
[!param](/Preconditioning/SMP/full) option set to `true` to build a fully
coupled Jacobian matrix. This helps the solver to better take the cross coupling
between displacement variables into account and will lead to improved
convergence.

### `Executioner`

Using the [!param](/Executioner/Transient/petsc_options_iname) and
[!param](/Executioner/Transient/petsc_options_value) parameters we can specify
pairs of PETSc options and their values. `-pctype lu` selects a direct LU
decomposition solver. It is a good choice for small problems  but does not provide
a lot of scalability for larger problems.

## Questions

Go ahead and run the input and visualize the result. Look at how the applied
boundary condition effect the deformation of the sample.

### Exploring parameters

> Experiment with different settings for the mechanical properties of the sample
> and the applied loading. What happens if you drastically reduce the Young's
> modulus or increase the applied pressure. Is the simulation result still valid?

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer02a.md)

### Units again

> What changes if you scale Young's and applied pressure by the same amount. Why?

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer02b.md)

### Sidebar: Automatic differentiation

In the current input file we are using only objects that provide a manually
derived implementation of their Jacobian matrix contribution. Such a derivation
is not always feasible, and it is not exact under every circumstance.

> If you created a large strain version of the input, try and convert it to use
> MOOSE's automatic differentiation system. A few places to look at:
>
> - [!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/use_automatic_differentiation) in the tensor mechanics master action
> - [!param](/BCs/Pressure/PressureAction/use_automatic_differentiation) in the Pressure BC action
> - [ADDirichletBC](ADDirichletBC.md)
> - [ADComputeIsotropicElasticityTensor](ComputeIsotropicElasticityTensor.md)
> - [ADComputeFiniteStrainElasticStress](ADComputeFiniteStrainElasticStress.md)

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer02c.md)

Once you've answered the questions and run this example we will move on to
[Step 3](tensor_mechanics/tutorials/introduction/step03.md) where the concept of subdomains or "blocks" is
introduced.
