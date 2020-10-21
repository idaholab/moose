# Example 8 : Material Properties

MOOSE includes built-in support for creating and sharing material properties thorughout your
simulation calculations. Material property calculations are run/updated by MOOSE automatically for
every quadrature point. Kernels, Postprocessors, and other objects all have convenient access to
these properties.  This example demonstrates a convection-diffusion problem with kernels that
utilize custom non-linear material properties.

Refer to the [Materials/index.md] documentation for more information about the material system.

## Problem Statement

This problem considers the same coupled system from [Example 3](examples/ex03_coupling.md):

!equation
\begin{aligned}
-\nabla \cdot \nabla u + \nabla\vec{v} \cdot \nabla u = 0 \\
-\alpha \nabla \cdot \nabla v = 0
\end{aligned}

but with slightly different boundary conditions: $u=v=0$ on the bottom boundary and $u=5$ and
$\nabla v=1$ on the top boundary. The remaining boundaries taking the natural boundary condition.
$\alpha$ is a diffusivity coefficient and $\nabla\vec{v}$ is a convection coefficient derived from
the coupled $v$ diffusion equation.

## Creating Material Objects

You create custom material properties by writing your own Material class:

!listing examples/ex08_materials/include/materials/ExampleMaterial.h start=#include end=private:

The `ExampleMaterial` object couples to the gradient of the "diffused" variable and uses this to
make the "convection_velocity" material property.  It also uses tabulated values specified in its
input file parameters and the z-coordinate of the current quadrature point to linearly interpolate
values for a "diffusivity" property.  We need to create member variables to hold the material
properties in addition to ones for helping compute those property values:

!listing examples/ex08_materials/include/materials/ExampleMaterial.h start=private: max-height=10000

Then we need to specify appropriate input file parameters for users and write code that retrieves
the data for use in calculations:

!listing examples/ex08_materials/src/materials/ExampleMaterial.C start=#include end=ExampleMaterial::computeQpProperties max-height=10000

The `computeQpProperties` function is where we put the code for actually calculating the material
property values. It will be automatically called by MOOSE at the right times and for each
quadrature point.  When we calculate a material property value,  we "set" it by storing the
calculated value in the member variable that was bound to the corresponding property in the
class's constructor (i.e.  _diffusivity and _convection_velocity):

!listing examples/ex08_materials/src/materials/ExampleMaterial.C re=void\sExampleMaterial::computeQpProperties.*$

## Plumbing Into Materials

In order to use the material properties we created, we need objects (e.g. our Kernels) to actually
support reading information from material properties.  For this problem, the `ExampleDiffusion`
kernel will use a "diffusivity" material property coefficient provided by our `ExampleMaterial`
class/object.  To do this we to have special code in three places:

- A member variable to store the material property value:

  ```
    const MaterialProperty<Real> & _diffusivity;
  ```

- A line in our constructor to bind our material property member to the value that is computed by
  the actual Material object:

  ```
  ExampleDiffusion::ExampleDiffusion(const InputParameters & parameters)
    : Diffusion(parameters), _diffusivity(getMaterialProperty<Real>("diffusivity"))
  {
  }
  ```

- Code that uses the material property to calculate something (e.g. our residual and jacobian):

  ```
  Real
  ExampleDiffusion::computeQpResidual()
  {
    return _diffusivity[_qp] * Diffusion::computeQpResidual();
  ...
  ```

Instead of directly coupling another variable into the convection kernel to use as the velocity
gradient term as in [Example 3](examples/ex03_coupling.md), we will instead use a
"convection_velocity" material property to provide the gradient in the `ExampleConvection` kernel.
Just like for `ExampleDiffusion` we make the three changes resulting in `ExampleConvection.C`
looking something like this:

!listing examples/ex08_materials/src/kernels/ExampleConvection.C start=::ExampleConvection

## Using Material Properties

Material properties now give us the flexibility to change/tweak our problem details without
requiring code modifications and compiling every time.  Changing how the convection velocity term
is computed requires nothing more than changing Material objects we are using. Different materials
can also be applied to different subdomains/blocks in your mesh. Let's see how we can use our
material properties in an input file:

!listing examples/ex08_materials/ex08.i block=Materials

In `ex08.i`, there are two material objects with each applied to a separate named subdomain of the
mesh via the `block = '...'` lines. These objects will provide the "diffused" and
"convection_velocity" properties that our convection and diffusion kernels now look for.

## Results

!media large_media/examples/ex08_convected.png
       caption=Convection
       style=width:49%;display:inline-flex;

!media large_media/examples/ex08_diffused.png
       caption=Diffusion
       style=width:49%;display:inline-flex;margin-left:2%

## Complete Source Files

- [examples/ex08_materials/ex08.i]
- [examples/ex08_materials/include/materials/ExampleMaterial.h]
- [examples/ex08_materials/src/materials/ExampleMaterial.C]
- [examples/ex08_materials/include/kernels/ExampleDiffusion.h]
- [examples/ex08_materials/src/kernels/ExampleDiffusion.C]
- [examples/ex08_materials/include/kernels/ExampleConvection.h]
- [examples/ex08_materials/src/kernels/ExampleConvection.C]

!content pagination use_title=True
                    previous=examples/ex07_ics.md
                    next=examples/ex09_stateful_materials.md
