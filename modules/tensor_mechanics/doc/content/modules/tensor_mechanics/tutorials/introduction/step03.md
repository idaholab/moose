# Step 3 - Subdomains and subdomain-specific properties

In this step we'll be setting up two subdomains (regions of our sample) with
differing material properties.

!listing modules/tensor_mechanics/tutorials/introduction/step03.i

## Input file

### `Mesh`

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
