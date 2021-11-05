# Step 1 - First Thermal Mortar Contact

Continuing from the [final step](contact/tutorials/introduction/step02.md) in
the mechanical contact tutorial, for the first time we incorporate thermal contact,
staying within the mortar method.

!listing modules/combined/tutorials/introduction/thermal_mechanical_contact/thermomech_cont_step01.i

Note that we do not have a "thermal contact action", so we need to add all
objects required for mortar based thermal contact manually.

## Input file

### `Variables`

For the thermal part of the problem we add the temperature `T` variable and supply an
`initial_condition` of 50 temperature units (this value is right between the 0
and 100 values we're setting below for the cooling and heating BCs). Note that
like the other physics variables and objects this variable is restricted to
block 0 (see `GlobalParams`).

The second variable we add is the Lagrange multiplier for the thermal contact.
Its physical meaning is the *heat flux*! This variable will be block restricted
to the lower dimensional mortar subdomain the `[Contact]` action created for us
(the name of that subdomain is the name of the contact action subblock with
`_secondary_subdomain` appended).

The [!param](/Variables/MooseVariable/family)
and [!param](/Variables/MooseVariable/order) parameters define the shape function
family and element order. First order Lagrange (no connection to "Lagrange multiplier!")
are the MOOSE default settings. In future inputs you might see those parameters
omitted. Lagrange shape functions are purely nodal interpolary shape functions.

### `Constraints`

The [`GapConductanceConstraint`](GapConductanceConstraint.md) is the bare bones
gap conductance constraint object available in MOOSE. It could serve as a
starting point for the development of more complex gap conductance models
(taking into account surface roughness and contact pressure for example).

The constraint object acts on the Lagrange multiplier
[!param](/Constraints/GapConductanceConstraint/variable) `Tlm`, and it affects
the [!param](/Constraints/GapConductanceConstraint/secondary_variable) `T`, the
temperature, enforcing the energy balance.

We make sure to enable the evaluation of the constraint on the *displaced mesh*
to capture large displacement and deformatio by setting the
[!param](/Constraints/GapConductanceConstraint/use_displaced_mesh) parameter to
`true`. The displaced mesh is a copy of the mesh that has all nodes moved by
their displacement variable values. Try setting this parameter to false and
compare the results (hint the gap distance the constraint sees will be computed
from the original mesh and will not change over time).

We chose a gap conductance [!param](/Constraints/GapConductanceConstraint/k).
The heat flux across the gap will be computed as $\frac kl$ where $l$ is the
width of the gap.

Then we set the primary and secondary boundaries and subdomains. Again for the
subdomains we choose the lower dimensional meshes that the mechanical contact
action set up for us. If you have a purely thermal contact problem you will need
to create those lower dimensional subdomains manually using the
[`LowerDBlockFromSidesetGenerator`](LowerDBlockFromSidesetGenerator.md) mesh
generator.

### `BCs`

`heat_left` and `cool_right` are two Dirichlet boundary conditions that set the
bottom of the left cantilever to an elevated temperature and the bottom of the
right cantilever to a lowered temperature, so we can establish a temperature
difference and a resulting heat flux between the two cantilevers.

### `Materials`

We need to add two material blocks for thermal properties.
[`HeatConductionMaterial`](HeatConductionMaterial.md) provides thermal
conductivity and specific heat. And the [`Density`](Density.md) material
computes an updated material density on the displaced mesh, taking volumetric
deformation into account. The `specific_heat` and `density` properties are used
by the [`HeatConductionTimeDerivative`](HeatConductionTimeDerivative.md) kernel,
and the `thermal_conductivity` property is used by the
[`HeatConduction`](HeatConduction.md) kernel.

### `Executioner`

We make a small change to the PETSc options and add the `-pc_factor_shift_type
nonzero` option in  [!param](/Executioner/Transient/petsc_options_iname) and
[!param](/Executioner/Transient/petsc_options_value) respectively. This helps
the direct solver deal with the saddle point structure of the Jacobian matrix
(with zeroes on the diagonal).

Note that we also lower the timestep to better see what is going on. You can set
it back to `0.5` to convince yourself that the problem still converges just as
well.

Once you've answered the questions and run this example we will move on to
[Step 2](combined/tutorials/introduction/step02.md) in which we introduce
a third block participating in the contact problem.
