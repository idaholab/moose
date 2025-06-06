# Step 10: Finite Volume id=step10

!---

We have only solved the heat equation in the concrete until now. We want to model natural circulation
in the water tank inside the shield.

Conservation of Mass:

!equation id=fluid_mass
\nabla \cdot \vec{u} = 0

Conservation of momentum:

!equation id=fluid_velocity
\frac{\partial \rho  \vec{u}}{\partial t} + \nabla \cdot \left(\rho \vec{u} \otimes \vec{u}\right)
= \nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u} + \left(\nabla \vec{u}\right)^T-\frac{2}{3}\nabla\cdot\vec{u}\mathbb{I}\right)\right) -\nabla p + \rho \vec{g}

Conservation of Energy:

!equation id=fluid_energy
\rho c_p \left( \frac{\partial T}{\partial t} + \vec{u}\cdot\nabla T \right) - \nabla\cdot k \nabla T = 0


where $\vec{u}$ is the fluid velocity, $\mu$ is fluid viscosity, $p$ is the pressure, $\rho$ is the density, $\vec{g}$ is the gravity vector, and $T$ is the temperature.

!! end-intro

!---

## Step 10: Natural convection

We introduce the Boussinesq approximation:

!equation
F_{buoyancy} = \alpha (T_w - T_{ref})

!---

We set up the flow equations:

!listing step10_finite_volume/step10.i

!---

## Step 10: Switch to 2D

While we can run 3D cases unconverged on a laptop, it is also clear the turn around
on finding issues is too long. We switch to 2D:

!row!

!col! width=50%

!media step10_mesh.png style=width:60%;

!col-end!

!col! width=50%

!style fontsize=80%
!listing step10_finite_volume/mesh2d.i

!col-end!

!row-end!

!---

## Variable Scaling

To make sure the convergence criterion is fairly applied to all equations, the non-linear variables
should be on the same scale.

Making equations non-dimensional is a common technique to achieve this. But this is not typically
done in MOOSE, where modelers have direct access to dimensionalized quantities.

MOOSE includes the ability to either manually or automatically scale non-linear variables.

!---

## Step 10: Run Input

```bash
moose-opt -i mesh2d.i --mesh-only
moose-opt -i step10.i
```

!---

## Step 10: Result

!media step10.png style=width:40%;margin-left:auto;margin-right:auto;display:block
