# Step 1 - First Contact

Continuing from the [final step](tensor_mechanics/tutorials/introduction/step04.md)
in the tensor mechanics tutorial we add a first attempt at mechanical contact.

!listing modules/contact/tutorials/introduction/step01.i

If you recall, in the final step of the mechanics tutorial we modeled two
cantilevers that were bent towards eachother and ended up occupying the same
space without interacting. We're about to change that.

## Input file

### `Mesh`

We add the [!param](/Mesh/FileMesh/patch_update_strategy) parameter and set it to
`iteration`. MOOSE actually recommends this setting when you run the model
without it. The parameter configures the geometric search that is required for
modeling contact.

### `Contact`

This is the major functional addition to the previous input. The
[`[Contact]`](Contact/index.md) action block is doing the heavy lifting for us
in setting up all objects required to enforce mechanical contact in one of the
numerous ways supported by MOOSE (penalty, kinematic, mortar - frictionless,
glued, frictional).

We use the [!param](/Contact/ContactAction/primary) and
[!param](/Contact/ContactAction/secondary) parameters to specify the two
interfaces we want to interact through mechanical contact. The
[!param](/Contact/ContactAction/model) parameter is set to select frictionless
contact and as the [!param](/Contact/ContactAction/formulation) we chose the
penalty method. Frictionless penalty based contact is a good initial choice for
contact modeling as it exhibits benign convergence properties and works in 2D as
well as 3D.

We select a [!param](/Contact/ContactAction/penalty) factor of 1e9. The choice
of the penalty should be guided by the stiffness of your system. A factor close
to the Young's modulus of the materials involved in contact, but not exceeding
it, is recommended. Penalty contact allows for a non-zero interpenetration of
the contact surfaces. Both the `kinematic` and `mortar` formulation will enforce
penetrations as low as the solution convergence allows.

We utilize the [!param](/Contact/ContactAction/normalize_penalty) option here to
compensate for mesh size effects.

### `BCs`

Note that we've made a small change to the pressure BC, changing the applied pressure from `1e4*t` to `1e4*t^2`. This is simply for demonstration purposes to apply a stronger pressure later in the simulation to force the cantilevers to bend more and establish contact over a larger area.

## Tasks and questions

First run the example and look at the output. You should see some new fields
that can be visualized in Paraview (or the Exodus viewer of your choice). In
particular please pay attention to `contact_force` and `penetration`.

### Penetration

> Visualize the penetration variable. You may have to rescale the visualization
> to start the color scale at 0. Negative penetrations are not of interest here
> (they effectively are the gap width)

You should see a maximum interpenetration of about 6e-4, which is about half a
percent of the element width. It is application dependent whether this amount is
acceptable.

Once you've answered the questions and run this example we will move on to
[Step 2](contact/tutorials/introduction/step02.md) which introduces
mortar based contact.
