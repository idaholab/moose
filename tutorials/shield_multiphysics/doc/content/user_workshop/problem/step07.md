# Step 7: Equation Coupling id=step07

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

## Step 7a: Solve heat equation only

We start simple by creating decoupled simulations.
The variables for the fluid solve are created but kept as auxiliary variables.

!listing step07_conjugate_heat_transfer/step7a_concrete_decoupled.i

!---

## Step 7b: Natural convection

We introduce the Boussinesq approximation:

!equation
F_{buoyancy} = \alpha (T_w - T_{ref})

!---

## Step 7b: Solve natural convection only

The `ConjugateHeatTransfer` object will only accept nonlinear variables for both
the solid and fluid temperature, so we create simpler boundary conditions for now.

!listing step07_conjugate_heat_transfer/step7b_water_decoupled.i

!---

## Variable Scaling

To make sure the convergence criterion is fairly applied to all equations, the non-linear variables
should be on the same scale.

Making equations non-dimensional is a common technique to achieve this. But this is not typically
done in MOOSE, where modelers have direct access to dimensionalized quantities.

MOOSE includes the ability to either manually or automatically scale non-linear variables.

!---

### Condition Number +without+ Scaling

The condition number of the Jacobian can be used to determine if variable scaling is required.

Using the step 7 executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step07_conjugate_heat_transfer
../executable/shield_multiphysics_opt -i step7b_water_decoupled.i Executioner/num_steps=1 Executioner/automatic_scaling=0 -pc_type svd -pc_svd_monitor
```

```bash
Time Step 1, time = 0.1, dt = 0.1

 0 Nonlinear |R| = 5.071028e+04
      SVD: condition number 1.165776227500e+11, 0 of 2490 singular values are (nearly) zero
      SVD: smallest singular values: 1.338172056200e-06 4.273321891855e-06 6.470317390884e-06 6.823445823264e-06 9.510905641771e-06
      SVD: largest singular values : 1.403643648521e+05 1.433352087376e+05 1.484948030786e+05 1.553257174022e+05 1.560009171422e+05
      0 Linear |R| = 5.071028e+04
      1 Linear |R| = 1.794202e-06
```

!---

### Condition Number +with+ Scaling

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step07_conjugate_heat_transfer
../executable/shield_multiphysics_opt -i step7b_water_decoupled.i Executioner/num_steps=1 Executioner/automatic_scaling=true -pc_type svd -pc_svd_monitor -ksp_view_pmat
```

```bash
Time Step 1, time = 0.1, dt = 0.1
 0 Nonlinear |R| = 2.510679e+04
      SVD: condition number 2.038170935394e+09, 0 of 2490 singular values are (nearly) zero
      SVD: smallest singular values: 2.662856988266e-06 8.978788356612e-06 1.424754114799e-05 1.508130484488e-05 2.196178581229e-05
      SVD: largest singular values : 4.786693460120e+03 4.873545109249e+03 4.874535146130e+03 5.190883760615e+03 5.427357718595e+03
      0 Linear |R| = 2.510679e+04
      1 Linear |R| = 1.820331e-07
 1 Nonlinear |R| = 2.777102e+03
```

!---

## Step 7b: Run decoupled

Using the tutorial executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step07_conjugate_heat_transfer
../executable/shield_multiphysics-opt -i step7b_water_decoupled.i
```

Using the conda MOOSE executable

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step07_conjugate_heat_transfer
moose-opt -i step7b_water_decoupled.i
```

!---

## Step 7c: Switch to 2D

While we can run these 3D cases unconverged on a laptop, it is also clear the turn around
on finding issues is too long. We switch to 2D:

!listing step07_conjugate_heat_transfer/step7c_2dcoupled.i

!---

Using the tutorial executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step07_conjugate_heat_transfer
../executable/shield_multiphysics-opt -i step7c_2dcoupled.i
```

Using the conda MOOSE executable

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step07_conjugate_heat_transfer
moose-opt -i step7c_2dcoupled.i
```

!---

## Step 7d: Run the 3D coupled simulation

Once we have found all the issues with the uncoupled simulations and the 2D coupled simulation, we can now run the 3D coupled simulation:

!listing step07_conjugate_heat_transfer/step7d_coupled.i

!---

Using the tutorial executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step07_conjugate_heat_transfer
mpirun -n 8 ../executable/shield_multiphysics-opt -i step7d_coupled.i
```

Using the conda MOOSE executable

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step07_conjugate_heat_transfer
mpirun -n 8 moose-opt -i step7d_coupled.i
```
