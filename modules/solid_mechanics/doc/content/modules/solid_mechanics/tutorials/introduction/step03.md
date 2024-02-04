# Step 3 - Subdomains and subdomain-specific properties

In this step we'll be setting up two subdomains (regions of our sample) with
differing material properties.

!listing modules/tensor_mechanics/tutorials/introduction/mech_step03.i

## Input file

### `Mesh`

Note that we refine the mesh a bit to better capture the discontinuity we're
introducing below.

The `block1` and `block2` subblocks are part of a chain of mesh generators,
linked by their `input` parameters. Each of the
[`SubdomainBoundingBoxGenerator`](SubdomainBoundingBoxGenerator.md) adds a
subdomain definition to the current mesh. Here we define two subdomains, one for
the left half of the domain and one for the right.

### `Materials`

We now define two elasticity tensors in this problem, one on the left half
(`block = 1`) and on on the right half (`block = 2`), referring to the subdomain
IDs we assigned using the mesh generators above.

Note how the stiffness of the right hand side is only half that of the left hand side.

### `Executioner`

We make a few changes in the Executioner block here, and you should try playing with some of the settings later on.

- We select NEWTON as our [!param](/Executioner/Transient/solve_type). This is a good (fast) option whenever we have a complete Jacobian for the system. It should give us 1-2 linear iterations for every non-linear iteration. Note that for NEWTON solves MOOSE automatically sets up an [`SMP`](SingleMatrixPreconditioner.md) with the [!param](/Preconditioning/SMP/full) option set to `true` (this can be disabled by setting [!param](/Executioner/auto_preconditioning) to `false`).
- We use LU decomposition to solve the linear problem, this preconditioner is very effective on a small problem like this. (For a more scalable preconditioner for large problems take a look at [HYPRE](application_development/hypre.md optional=true).)

## Questions

### Visualizing strain

So far we've only looked at the deformation of the mesh. MOOSE can visualize a
host of mechanical quantities, and the master action makes this particularly
easy.

> Try and add output for the vonMises stress in the simulation domain. Take a
> look at the
> [!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/generate_output)
> parameter...

[Click here for the answer.](tensor_mechanics/tutorials/introduction/answer03a.md)

### Sidebar: Thermal expansion

> In addition to externally applied loading deformation can be induced by
> internal changes of a material. One common effect is the thermal expansion (or
> contraction) under temperature changes.

[Click here for the sidebar on thermal expansion.](tensor_mechanics/tutorials/introduction/step03a.md)

Once you've answered the questions and run this example we will move on to
[Step 4](tensor_mechanics/tutorials/introduction/step04.md) and setup a cantilever problem that prepares us
for contact.
