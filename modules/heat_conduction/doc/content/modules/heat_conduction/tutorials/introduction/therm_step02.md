# Step 2 - Adding boundary conditions

In the previous step we set up a basic thermal simulation that
did... nothing. In this step will prescribe values for the temperature
on the left and right sides of the block.

!listing modules/heat_conduction/tutorials/introduction/therm_step02.i

## Input file

### `BCs`

BCs stands for *boundary conditions*. Those apply to the boundaries (or
sidesets) of the simulation domain. In all boundary condition objects you will
see the mandatory `boundary` parameter, which expects a list of sideset names or
IDs.

!listing modules/heat_conduction/tutorials/introduction/therm_step02.i block=BCs

#### `DirichletBC`

The two Dirichlet boundary conditions are set on the
*left* and *right* surfaces of the simulation domain. These prescribe the values
of the `T` variable on those surfaces. The `GeneratedMeshGenerator` that was used
to create this simple block mesh defines boundaries named `top`, `bottom`, `left`,
and `right` on the boundaries of that block. Additional boundaries can be added
using other MeshGenerators, or boundaries can be defined with an external meshing
tool.

In this case, the temperature on the left boundary is fixed to a constant value
of 300 using the [`DirichletBC`](DirichletBC.md) 

#### `FunctionDirichletBC`

The temperature on the right boundary is defined using a time-dependent function
using the [`FunctionDirichletBC`](FunctionDirichletBC.md). As its name implies,
this is a Dirichlet boundary condition that is defined using a function rather
than a constant value. In this case, an expression for an analytic function is
provided for the `function` parameter. Alternatively, the name of a separately
defined `Function` can be provided. A wide variety of function types 
[is available](heat_conduction/tutorials/introduction/supplemental02a.md).

#### Other boundary conditions

A number of [other boundary conditions](heat_conduction/tutorials/introduction/supplemental02b.md)
are available. Some of these are generic boundary conditions that can be applied
to any problem, while some are physics-specific.

## Questions

Before running the model, consider what the expected solution to this problem
with only the conduction term should be.

[Click here for the answer.](heat_conduction/tutorials/introduction/answer02a.md)

Now go ahead and run the input and visualize the result to see if it matches
the behavior you would expect.

### Exploring parameters

Experiment with different settings for the thermal conductivity of the material
and the applied boundary conditions. What happens if you change the thermal
conductivity?

[Click here for the answer.](heat_conduction/tutorials/introduction/answer02b.md)

### Sidebar: 

> For a problem like this, it can be very helpful to plot out values of the solution
> sampled along a line.

[Click here for the sidebar on plotting along a line.](heat_conduction/tutorials/introduction/therm_step02a.md)

Once you've answered the questions and run this example we will move on to
[Step 3](heat_conduction/tutorials/introduction/therm_step03.md), where additional terms will be
added to the heat equation.
