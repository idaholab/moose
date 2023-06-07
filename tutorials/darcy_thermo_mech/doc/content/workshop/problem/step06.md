# Step 6: Equation Coupling id=step06

!---

The pressure and heat equations have been developed independently up to this point, in this step,
they are coupled together.

!equation
-\nabla \cdot \frac{\mathbf{K}}{\mu} \nabla p  = 0
\\
C\left( \frac{\partial T}{\partial t} + \underbrace{\epsilon \vec{u}\cdot\nabla T}_{\textrm{DarcyAdvection}} \right) - \nabla \cdot k \nabla T = 0

- Objects have been created for everything except the $\vec{u}\cdot\nabla T$ term; a `Kernel`,
  `DarcyAdvection`, will be developed for this term.
- A more sophisticated `Material` object will be created that includes temperature dependence.

!---

## DarcyAdvection.h

!listing step06_coupled_darcy_heat_conduction/include/kernels/DarcyAdvection.h

!---

## DarcyAdvection.C

!listing step06_coupled_darcy_heat_conduction/src/kernels/DarcyAdvection.C

!---

## LinearInterpolation

MOOSE contains several utilities and classes to assist in performing calculations. One of these is
the `LinearInterpolation` class.

This class allows the construction of a linear interpolation of input data, and the sampling of that
interpolation (values, derivatives, integral, etc.) within a MOOSE object. Linear extrapolation beyond
the bounds of the interpolated function is also possible.

This feature is used in this step to improve the manual interpolation performed previously.

!---

## LinearInterpolation

This utility allows the manual permeability interpolation in `PackedColumn.C`:

```c++
  _permeability[_qp] =
      (permeability[0] * (sphere_sizes[1] - value) + permeability[1] * (value - sphere_sizes[0])) /
      (sphere_sizes[1] - sphere_sizes[0]);
```

to be performed automatically, being initialized once in the constructor:

```c++
  _permeability_interpolation.setData(sphere_sizes, permeability);
```

with sampling as necessary in `PackedColumn::computeQpProperties`:

```c++
  _permeability[_qp] = _permeability_interpolation.sample(value);
```

`LinearInterpolation` can be used extensively to interpolate manually entered data, as well as
when using imported input data.

!---

## PackedColumn.h

!listing step06_coupled_darcy_heat_conduction/include/materials/PackedColumn.h

!---

## PackedColumn.C

!listing step06_coupled_darcy_heat_conduction/src/materials/PackedColumn.C

!---

## HeatConductionOutflow.h

!listing step06_coupled_darcy_heat_conduction/include/bcs/HeatConductionOutflow.h

!---

## HeatConductionOutflow.C

!listing step06_coupled_darcy_heat_conduction/src/bcs/HeatConductionOutflow.C

!---

## Step 6a: Coupled Pressure and Heat Equations

!listing step06_coupled_darcy_heat_conduction/problems/step6a_coupled.i

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

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step06_coupled_darcy_heat_conduction
make -j 12
../darcy_thermo_mech-opt -i step6a_coupled.i Mesh/nx=50 Mesh/ny=3 Executioner/num_steps=1 Executioner/automatic_scaling=0 -pc_type svd -pc_svd_monitor -ksp_view_pmat
```

```bash
Time Step 1, time = 0.1, dt = 0.1

 0 Nonlinear |R| = 8.000625e+03
      SVD: condition number 2.835686200265e+15, 2 of 408 singular values are (nearly) zero
      SVD: smallest singular values: 1.434461194336e-13 5.583840793234e-13 1.222432395761e-12 2.076808734751e-12 3.047037450013e-12
      SVD: largest singular values : 4.006299266699e+02 4.029206639889e+02 4.047115548038e+02 4.059957077255e+02 4.067681813595e+02
      0 Linear |R| = 8.000625e+03
      1 Linear |R| = 8.101613e-09
Mat Object: () 1 MPI processes
  type: seqaij
row 0: (0, 1.)  (1, 0.)  (2, 0.)  (3, 0.)  (4, 0.)  (5, 0.)  (6, 0.)  (7, 0.)
row 1: (0, 0.)  (1, 1.)  (2, 0.)  (3, 0.)  (4, 0.)  (5, 0.)  (6, 0.)  (7, 0.)
row 2: (0, 1.32667e-12)  (1, 0.)  (2, -1.07325e-11)  (3, 0.)  (4, 3.97056e-14)  (5, 0.)  (6, 4.01973e-12)  (7, 0.)  (8, 1.32667e-12)  (9, 0.)  (10, 4.01973e-12)  (11, 0.)
row 3: (0, -2.81185e-20)  (1, 3.41152)  (2, -1.12474e-19)  (3, 14.0732)  (4, 1.12474e-19)  (5, 13.7863)  (6, 2.81185e-20)  (7, 3.33981)  (8, -2.81185e-20)  (9, 3.41152)  (10, 2.81185e-20)  (11, 3.33981)
row 4: (0, 4.01973e-12)  (1, 0.)  (2, 3.97056e-14)  (3, 0.)  (4, -6.43156e-11)  (5, 0.)  (6, 1.59995e-11)  (7, 0.)  (8, 4.01973e-12)  (9, 0.)  (10, 1.59995e-11)  (11, 0.)  (204, 1.19117e-13)  (205, 0.)  (206, 1.20592e-11)  (207, 0.)  (208, 1.20592e-11)  (209, 0.)
```

!---

### Condition Number +with+ Scaling

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step06_coupled_darcy_heat_conduction
make -j 12
../darcy_thermo_mech-opt -i step6a_coupled.i Mesh/nx=50 Mesh/ny=3 Executioner/num_steps=1 -pc_type svd -pc_svd_monitor -ksp_view_pmat
```

```bash
Time Step 1, time = 0.1, dt = 0.1

 0 Nonlinear |R| = 8.000625e+03
      SVD: condition number 2.877175736279e+04, 0 of 408 singular values are (nearly) zero
      SVD: smallest singular values: 1.413775933915e-02 5.524422458767e-02 1.194077235260e-01 2.001521770346e-01 2.889664356969e-01
      SVD: largest singular values : 4.006299266699e+02 4.029206639889e+02 4.047115548038e+02 4.059957077255e+02 4.067681813595e+02
      0 Linear |R| = 8.000625e+03
      1 Linear |R| = 3.858046e-09
Mat Object: () 1 MPI processes
  type: seqaij
row 0: (0, 1.)  (1, 0.)  (2, 0.)  (3, 0.)  (4, 0.)  (5, 0.)  (6, 0.)  (7, 0.)
row 1: (0, 0.)  (1, 1.)  (2, 0.)  (3, 0.)  (4, 0.)  (5, 0.)  (6, 0.)  (7, 0.)
row 2: (0, 0.132667)  (1, 0.)  (2, -1.07325)  (3, 0.)  (4, 0.00397056)  (5, 0.)  (6, 0.401973)  (7, 0.)  (8, 0.132667)  (9, 0.)  (10, 0.401973)  (11, 0.)
row 3: (0, -2.81185e-20)  (1, 3.41152)  (2, -1.12474e-19)  (3, 14.0732)  (4, 1.12474e-19)  (5, 13.7863)  (6, 2.81185e-20)  (7, 3.33981)  (8, -2.81185e-20)  (9, 3.41152)  (10, 2.81185e-20)  (11, 3.33981)
row 4: (0, 0.401973)  (1, 0.)  (2, 0.00397056)  (3, 0.)  (4, -6.43156)  (5, 0.)  (6, 1.59995)  (7, 0.)  (8, 0.401973)  (9, 0.)  (10, 1.59995)  (11, 0.)  (204, 0.0119117)  (205, 0.)  (206, 1.20592)  (207, 0.)  (208, 1.20592)  (209, 0.)
```

!---

## Step 6a: Run

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step06_coupled_darcy_heat_conduction
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step6a_coupled.i
```

!---

## Step 6a: Results

!media darcy_thermo_mech/step06a_result.webm

!---

## Step 6b:\\ Oscillating Pressure and Temperature Dependence

Vary the inlet and output pressure:

- Inlet (left): $p = 2000\sin(0.466\pi t)$
- Outlet (right): $p = 2000\cos(0.466\pi t)$

Viscosity, density, thermal conductivity, and specific heat capacity of the fluid are setup to vary
as a function of temperature.


!---

## Step 6b: Input File

!listing step06_coupled_darcy_heat_conduction/problems/step6b_transient_inflow.i

!---

## Step 6b: Run

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step06_coupled_darcy_heat_conduction
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step6b_transient_inflow.i
```

!---

## Step 6b: Results

!media darcy_thermo_mech/step06b_result.mp4

!---

## Decoupling Heat Equation

Pressure is changed to a constant linearly varying auxiliary variable. We only solve for velocity

!listing step06_coupled_darcy_heat_conduction/problems/step6c_decoupled.i
