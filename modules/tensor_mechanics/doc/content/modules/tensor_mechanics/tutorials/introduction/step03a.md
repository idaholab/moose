# Step 3a - Thermal expansion

In this sidebar we'll introduce a thermal expansion eigenstrain. To demonstrate
the effect we reshape the domain from step 3 and make it narrow like a
cantilever.

!listing modules/tensor_mechanics/tutorials/introduction/mech_step03a.i

## Input file

### `Mesh`

Note the xmin and xmax upper *and* lower x-dimension extents are supplied here.
The origin will lie in the center of the bottom boundary of this generated mesh.

New is the [`ExtraNodesetGenerator`](ExtraNodesetGenerator.md) which we append
to the existing chain of mesh generators. This generator allows us to create a
new nodeset containing all nodes located at the coordinates the user specified in
[!param](/Mesh/ExtraNodesetGenerator/coord). Here we create a nodeset containing
the single node at the origin. You need to make sure that a node actually exists
at each specified coordinate! We call this nodeset `pin` for obvious reasons and
will use it below in the BCs.

### `AuxVariables`

We introduce a new auxiliary variable `T` (for temperature). Auxiliary variables
are variables we're not solving for, but are computing directly. To simplify
this step we are not solving a heat conduction problem, but instead just
prescribing a global temperature that is rising with time (see next section).
Auxiliary variables can be coupled in everywhere regular (so called *nonlinear*)
variables are coupled. They are a great tool for simplifying a model during
development.

### `AuxKernels`

The [`FunctionAux`](FunctionAux.md) AuxKernel can set an AuxVariable to a
function of space and time. Note the
[!param](/AuxKernels/FunctionAux/execute_on) parameter that is available in many
MOOSE systems. Here we skip execution during LINEAR and NON_LINEAR iterations
and only update the variable value at the beginning of the timestep.

### TensorMechanics `Master` Action

We've added the
[!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/automatic_eigenstrain_names)
parameter to the master action. With this option enabled the master action will
try to automatically detect all material objects that provide eigenstrain
properties. This works well for most scenarios. Note that MOOSE will print a
list of detected eigenstrain names very early in its console output. Look for

```
*** Automatic Eigenstrain Names ***
all: thermal_expansion
```

when you run this example. Here `all` is the master action block name and
`thermal_expansion` is the
[!param](/Materials/ComputeThermalExpansionEigenstrain/eigenstrain_name)
parameter value for the two eigenstrain materials below. The action correctly
detected it and verified that eigenstrains are provided on all subdomains
covered by the master action block. To manually supply the eigenstrain material
properties use the
[!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/eigenstrain_names)
parameter. Like so

```
[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    eigenstrain_names = 'thermal_expansion'
    generate_output = 'vonmises_stress'
  []
[]
```

### `BCs`

We have changed the way we apply the boundary condition on the x displacement
variable `disp_x` to only constrain this variable at the `pin` "boundary". Above
we defined this boundary as the nodeset containing the single node at (0,0,0).

Pinning at a single point is less restrictive and allows the model to expand in
x-direction along the bottom boundary (which will happen due to isotropic
thermal expansion). Yet we still remove all rigid body modes

- Translation, because we fix both x and y displacement on *at least* one node
- Rotation, because we fix y displacement on another node (actually on all nodes along the bottom boundary)

Pinning nodes to remove rigid body modes is an important tool to create
mechanics simulations that converge. The presence of rigid body modes will lead
to **non-convergence**. BCs on single nodes rather than whole boundaries can help
avoid overconstraining your problems. Keep in mind that to remove all 6 rigid
body modes in a 3D simulation you need to apply BCs on at least 3 nodes (which
cannot be co-linear). One node will have to be constrained in 3 direction, one
in 2 and one in just one direction. The first node will remove three translation
modes. The second node will remove two rotational modes (and will establish an
axis of rotation). The third node will remove that final rotational mode.

### `Materials`

An "Eigenstrain" in MOOSE is a stress free strain. It is an intrinsic shape
change of a volume element due to effects like thermal expansion, swelling,
diffusion of over/undersized solutes, etc.

We use two
[`ComputeThermalExpansionEigenstrain`](ComputeThermalExpansionEigenstrain.md)
objects to compute an eigenstrain tensor in each of the two subdomains. The
[!param](/Materials/ComputeThermalExpansionEigenstrain/stress_free_temperature)
is set to 300K, which is the initial temperature set for the `T` AuxVariable. At
this temperature we assume the eigenstrain to be zero (with contraction at
temperatures lower than 300K and expansion above).  The
[!param](/Materials/ComputeThermalExpansionEigenstrain/thermal_expansion_coeff)
is chosen differently on the two subdomains. This effectively models a
bimetallic strip.

## Questions

### Expected outcome

> Think about what you expect to happen when you run the input.

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer03b.md)

### Overconstraining

> Apply the `disp_x` boundary condition to the entire bottom surface again and
> observe what happens. Undo that change before you move on to the next
> question.

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer03c.md)

### Constraining even less

> In the original input we're fixing the y displacement to 0 in the entire
> bottom boundary. Try to relax this constraint a bit try to add a second single
> node boundary using the `ExtraNodesetGenerator` (chained in after the `pin`
> generator). Use one of the two bottom corner nodes. Now think about where you
> have to apply the `disp_y != 0` boundary condition.

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer03d.md)

Once you've answered the questions and run this example we will move on to
[Step 4](tensor_mechanics/tutorials/introduction/step04.md)  and setup a
cantilever problem that prepares us for contact.
