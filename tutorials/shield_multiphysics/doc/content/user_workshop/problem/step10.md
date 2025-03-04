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
= \nabla \cdot \left(\mu_\text{eff} \left(\nabla\vec{u}_I + \left(\nabla \vec{u}_I\right)^T-\frac{2}{3}\nabla\cdot\vec{u}_I\mathbb{I}\right)\right) -\nabla p + \rho \vec{g}

Conservation of Energy:

!equation id=fluid_energy
\rho c_p \left( \frac{\partial T}{\partial t} + \vec{u}\cdot\nabla T \right) - \nabla\cdot k \nabla T = 0


where $\vec{u}$ is the fluid velocity, $\mu$ is fluid viscosity, $p$ is the pressure, $\rho$ is the density, $\vec{g}$ is the gravity vector, and $T$ is the temperature.

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

!media step10_mesh.png

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

### Condition Number +without+ Scaling

The condition number of the Jacobian can be used to determine if variable
scaling is required. (This command will take a while.)

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step10_finite_volume
moose-opt -i step10.i Executioner/num_steps=1 Executioner/automatic_scaling=0 -pc_type svd -pc_svd_monitor
```

```text
Time Step 1, time = -0.9, dt = 0.1
 0 Nonlinear |R| = 1.555021e+03
      SVD: condition number 2.487578074181e+10, 0 of 1313 singular values are (nearly) zero
      SVD: smallest singular values: 7.611317045923e-09 1.708134680767e-08 4.190012116989e-08 1.010995179237e-07 1.374565073482e-07
      SVD: largest singular values : 1.824098672066e+02 1.870387496055e+02 1.871937194624e+02 1.888655324055e+02 1.893374539908e+02
      0 Linear |R| = 1.555021e+03
      1 Linear |R| = 7.108241e-08
```

!---

### Condition Number +with+ Scaling

```bash
moose-opt -i step10.i Executioner/num_steps=1 Executioner/automatic_scaling=true -pc_type svd -pc_svd_monitor -ksp_view_pmat
```

```text
Time Step 1, time = -0.9, dt = 0.1

Performing automatic scaling calculation

 0 Nonlinear |R| = 1.025510e+01
      SVD: condition number 1.471119164321e+07, 0 of 1313 singular values are (nearly) zero
      SVD: smallest singular values: 1.248782233114e-06 4.015976514919e-06 9.864276118040e-06 2.246290567975e-05 2.535962989263e-05
      SVD: largest singular values : 1.767452311492e+01 1.767930840726e+01 1.768272100899e+01 1.768476342681e+01 1.837107475197e+01
      0 Linear |R| = 1.025510e+01
      1 Linear |R| = 1.388368e-09
```

!---

## Step 10: Run Input

```bash
moose-opt -i step10.i
```

!---

## Step 10: Result

!media step10.png style=width:40%;margin-left:auto;margin-right:auto;display:block
