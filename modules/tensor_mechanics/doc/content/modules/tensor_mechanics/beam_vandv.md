# Beams

## Small strain Timoshenko beam bending

A 1D cantilever beam of 4 m is modeled using beam elements in MOOSE. The beam has a cross section area of 0.554 ${m}^2$, moment of inertia about Y and Z axis of 0.0142 ${m}^4$, Young's modulus of elasticity of 2.6 N/${m}^2$, poisson's ratio of 0.3, shear modulus of 1 N/${m}^2$ and shear coefficient of 0.85. A point load of magnitude 1$\times$10$^-$$^4$ N is applied at the free end of the cantilever beam in Y-direction.

!listing timoshenko_small_strain_y.i


### Results

For this beam, the dimensionless parameter alpha is given by:
\begin{equation}
alpha = \frac {kAGL^2}{EI} = 204.3734
\end{equation}
The value of alpha is not high  enough for the beam to behave like a thin beam where shear effects are not significant. Hence, the shear effects are considered and the small deformation analytical deflection of a cantilever beam is given by:
\begin{equation}
\Delta = \frac {PL^3}{3EI} \; (1 + \frac {3}{alpha}) = 5.868 \times 10^{-2} m
\end{equation}

The deflection obtained from MOOSE using 10 elements is 5.852$\times$10$^-$$^2$ m shown in [fig:timoshenko_small_strain2].

!media media/tensor_mechanics/vandv/timoshenko_small_strain2.png
       style=width:450px;margin-left:110px;float:center;
       id=fig:timoshenko_small_strain2
       caption=Displacement of the Timoshenko beam in bending.

## Small strain Euler beam bending

A 1D cantilever beam of 4 m is modeled using beam elements in MOOSE. The beam has a cross section area of 0.554 ${m}^2$, moment of inertia about Y and Z axis of 0.0142 ${m}^4$, Young's modulus of elasticity of 2.6 N/${m}^2$, poisson's ratio of -0.99, shear modulus of 1$\times$10$^4$ N/${m}^2$ and shear coefficient of 0.85. A point load of magnitude 1$\times$10$^-$$^4$ N is applied at the free end of the cantilever beam in Y-direction.

!listing euler_small_strain_y.i

### Results

For this beam, the dimensionless parameter alpha is given by:
\begin{equation}
alpha = \frac {kAGL^2}{EI} = 2.04 \times 10^6
\end{equation}
Since the value of alpha is quite high, the beam behaves like a thin beam where shear effects are not significant. Hence, the small deformation analytical deflection of a cantilever beam is given by:
\begin{equation}
\Delta = \frac {PL^3}{3EI} \; (1 + \frac {3}{alpha}) = \frac {PL^3}{3EI} \; =5.78 \times 10^{-2} m
\end{equation}

The deflection obtained from MOOSE using 10 elements is 5.766$\times$10$^-$$^2$ m shown in [fig:euler_small_strain2]. The ratio beam FEM solution and analytical solution is 0.998.

!media media/tensor_mechanics/vandv/euler_small_strain2.png
       style=width:450px;margin-left:110px;float:center;
       id=fig:euler_small_strain2
       caption=Displacement of the Euler beam in bending.

## Small strain Euler beam axial loading

A pipe 5 feet (60 inches) long having an internal diameter of 8 inches and outer diameter of 10 inches is modeled using beam elements in MOOSE. The Young's modulus of elasticity of the pipe is 30$\times$10$^6$ lb/${in}^2$, the shear modulus is 11.54$\times$10$^6$ lb/${in}^2$ and poisson's ratio is 0.3. The pipe is fixed at one end and an axial load of 50000 lb is applied at the other end.

!listing euler_pipe_axial_force.i

### Results

The analytical displacement at the end is given by:
\begin{equation}
\Delta = \frac {PL}{AE} = 3.537 \times 10^{-3} in
\end{equation}

The displacement at the end obtained from the MOOSE using 10 elements is 3.537$\times$10$^-$$^3$ in as shown in [fig:euler_pipe_axial_force].

!media media/tensor_mechanics/vandv/euler_pipe_axial_force2.png
       style=width:450px;margin-left:110px;float:center;
       id=fig:euler_pipe_axial_force
       caption=Displacement of the Euler beam under axial load.

## Large strain/large rotation of cantilever beam

A 1D cantilever beam of 4 m is modeled using beam elements in MOOSE. The beam has a cross section area of 1 ${m}^2$, moment of inertia about Y and Z axis of 0.16 ${m}^4$, Young's modulus of elasticity of 1$\times$10$^4$ N/${m}^2$, poisson's ratio of -0.99, shear modulus of 1$\times$10$^8$ N/${m}^2$ and shear coefficient of 1. A point load of magnitude 300 N is applied at the free end of the cantilever beam in Y-direction.

!listing euler_finite_rot_y.i

### Results

For this beam, the dimensionless parameter alpha is given by:
\begin{equation}
alpha = \frac {kAGL^2}{EI} = 1\times 10^6
\end{equation}
 Since, the value of alpha is quite high, the beam behaves like a thin beam where shear effects are not significant. The analytical solution for the displacements due to large rotations and large strain of the cantilever beam in X and Y direction is -1 m and 2.4 m respectively ([!citet](bishopanddrucker1945)).

The displacement at the free end of the cantilever beam obtained from the MOOSE  is -0.954 m in X direction as shown in [fig:euler_finite_rot_x] and 2.37 m in Y direction as shown in [fig:euler_finite_rot_y].


!media media/tensor_mechanics/vandv/euler_finite_rot_x2.png
       style=width:450px;margin-left:110px;float:center;
       id=fig:euler_finite_rot_x
       caption=Displacement of the cantilever beam with large strain and rotation in X direction


!media media/tensor_mechanics/vandv/euler_finite_rot_y2.png
       style=width:450px;margin-left:110px;float:center;
       id=fig:euler_finite_rot_y
       caption=Displacement of the cantilever beam with large strain and rotation in Y direction


## Torsion

A 1D cantilever beam of 1 m is modeled using beam elements in MOOSE. The beam has a cross section area of 0.5 ${m}^2$, moment of inertia about Y and Z axis of 1$\times$10$^-$$^5$ ${m}^4$, Young's modulus of elasticity of 2$\times$10$^9$ N/${m}^2$, poisson's ratio of 0.3 and shear coefficient of 1. A torsion of magnitude 5 N/m is applied at the free end of the cantilever beam.

!listing torsion_1.i

### Results

The analytical solution for the axial twist at the free end of the beam is given by:
\begin{equation}
\phi = \frac {TL}{GI_{x}} = 3.25\times 10^{-4} rad
\end{equation}
\begin{equation}
where, I_{x} = I_{y} + I_{z} = 2\times 10^{-5} m^4
\end{equation}

The rotation at the free end of the beam obtained in MOOSE is 3.2$\times$10$^-$$^4$ rad, as shown in [fig:torsion2].

!media media/tensor_mechanics/vandv/torsion2.png
       style=width:450px;margin-left:110px;float:center;
       id=fig:torsion2
       caption=Rotation of the beam due to torsion.

## Small strain Euler beam vibration

A 1D cantilever beam of 4 m is modeled using beam elements in MOOSE. The beam has a cross section area of 0.01 ${m}^2$, moment of inertia about Y and Z axis of 1$\times$10$^-$$^4$ ${m}^4$, Young's modulus of elasticity of 1$\times$10$^4$ N/${m}^2$, poisson's ratio of -0.99, density of 1 kg/${m}^3$, shear modulus of 4$\times$10$^7$ N/${m}^2$ and shear coefficient of 1. An impulse load with a peak value of 0.01 N at 0.05 s, as shown in [fig:impulseload], is applied at the free end of the beam in Y direction. The Newmark time integration parameters used in the problem correspond to the Newmark's average acceleration method, i.e., `beta=0.25` and `gamma=0.5`.

!media media/tensor_mechanics/vandv/euler_small_impulse.png
       style=width:500px;margin-left:110px;float:center;
       id=fig:impulseload
       caption=Impulse load pattern applied at the free end of the beam.

!listing dyn_euler_small.i

### Results

For this beam, the dimensionless parameter alpha is given by:
\begin{equation}
alpha = \frac {kAGL^2}{EI} = 6.4\times 10^6
\end{equation}
 Therefore, the beam behaves like an Euler-Bernoulli beam. The displacement, velocity and acceleration, as the function of time, at the free end of the cantilever beam in MOOSE, are compared with the results from ABAQUS using the time step of 0.05 s, as shown in [fig:euler_vibration_disp], [fig:euler_vibration_vel] and [fig:euler_vibration_acc].

!media media/tensor_mechanics/vandv/euler_small_disp.png
       style=width:500px;margin-left:110px;float:center;
       id=fig:euler_vibration_disp
       caption=Displacement at the free end of the Euler beam.

!media media/tensor_mechanics/vandv/euler_small_vel.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:euler_vibration_vel
      caption=Velocity at the free end of the Euler beam.

!media media/tensor_mechanics/vandv/euler_small_acc.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:euler_vibration_acc
      caption=Acceleration at the free end of the Euler beam.


## Small strain Timoshenko beam vibration

A 1D cantilever beam of 4 m is modeled using beam elements in MOOSE. The beam has a cross section area of 1 ${m}^2$, moment of inertia about Y and Z axis of 1 ${m}^4$, Young's modulus of elasticity of 2$\times$10$^4$ N/${m}^2$, poisson's ratio of -0.99, density of 1 kg/${m}^3$, shear modulus of 1$\times$10$^4$ N/${m}^2$ and shear coefficient of 1. An impulse load with a peak value of 0.01 N at 0.0.005 s, as shown in [fig:impulseload2], is applied at the free end of the beam in Y direction. The Newmark time integration parameters used in the problem correspond to the Newmark's average acceleration method, i.e., `beta=0.25` and `gamma=0.5`

!media media/tensor_mechanics/vandv/timo_small_impulse.png
       style=width:500px;margin-left:110px;float:center;
       id=fig:impulseload2
       caption=Impulse load pattern applied at the free end of the beam.

!listing dyn_timoshenko_small.i

### Results

For this beam, the dimensionless parameter alpha is given by:
\begin{equation}
alpha = \frac {kAGL^2}{EI} = 8
\end{equation}
 Therefore, the beam behaves like a Timoshenko beam. The displacement, velocity and acceleration, as the function of time, at the free end of the cantilever beam in MOOSE, are compared with the results from ABAQUS using the time step of 0.005 s, as shown in [fig:timo_vibration_disp], [fig:timo_vibration_vel] and [fig:timo_vibration_acc].

!media media/tensor_mechanics/vandv/timoshenko_small_disp.png
       style=width:500px;margin-left:110px;float:center;
       id=fig:timo_vibration_disp
       caption=Displacement at the free end of the Timoshenko beam.

!media media/tensor_mechanics/vandv/timoshenko_small_vel.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:timo_vibration_vel
      caption=Velocity at the free end of the Timoshenko beam.

!media media/tensor_mechanics/vandv/timoshenko_small_acc.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:timo_vibration_acc
      caption=Acceleration at the free end of the Timoshenko beam.


## Small strain massless beam vibration with a lumped mass

A 1D cantilever beam of 4 m is modeled using beam elements in MOOSE. The beam is massless with a lumped mass of 0.0189972 kgs at its free end. The beam has a moment of inertia of 1$\times$10$^-$$^4$ ${m}^4$ about Y and Z axis, Young's modulus of elasticity of 1$\times$10$^4$ N/${m}^2$, poisson's ratio of -0.99, shear modulus of  4$\times$10$^7$ N/${m}^2$ and shear coefficient of 1. An impulse load with a peak value of 0.01 N at 0.1 s, as shown in [fig:impulseload3], is applied at the free end of the beam in Y direction. The Newmark time integration parameters used in the problem correspond to the Newmark's average acceleration method, i.e., `beta=0.25` and `gamma=0.5`.

!media media/tensor_mechanics/vandv/massless_small_impulse.png
       style=width:500px;margin-left:110px;float:center;
       id=fig:impulseload3
       caption=Impulse load pattern applied at the free end of the beam.

!listing dyn_euler_small_added_mass.i

### Results

The displacement, velocity and acceleration, as the function of time, at the free end of the cantilever beam in MOOSE, are compared with the results from ABAQUS using the time step of 0.1 s, as shown in [fig:added_mass_disp], [fig:added_mass_vel] and [fig:added_mass_acc].

!media media/tensor_mechanics/vandv/added_mass_disp.png
       style=width:500px;margin-left:110px;float:center;
       id=fig:added_mass_disp
       caption=Displacement at the free end of the massless beam with lumped mass.

!media media/tensor_mechanics/vandv/added_mass_vel.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:added_mass_vel
      caption=Velocity at the free end of the massless beam with lumped mass.

!media media/tensor_mechanics/vandv/added_mass_acc.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:added_mass_acc
      caption=Acceleration at the free end of the massless beam with lumped mass.

## Small strain massless beam damped vibration with a lumped mass having rotational moment of inertia

A 1D cantilever beam of 4 m is modeled using beam elements in MOOSE. The beam is massless with a lumped mass of 0.01899772 kgs at its free end whose moment of inertia about X, Y and Z axis is 0.2 ${m}^4$,0.1 ${m}^4$ and 0.1 ${m}^4$, . The beam has a moment of inertia about Y and Z axis of 1$\times$10$^-$$^4$ ${m}^4$, Young's modulus of elasticity of 1$\times$10$^4$ N/${m}^2$, poisson's ratio of -0.99, shear modulus of  4$\times$10$^7$ N/${m}^2$ and shear coefficient of 1. An impulse load with a peak value of 0.01 N at 0.1 s as shown in [fig:impulseload4] is applied at the free end of the beam in Y direction. The Newmark time integration parameters used in the problem correspond to the Newmark's average acceleration method, i.e., `beta=0.25` and `gamma=0.5` and vibration is damping using the mass proportional coefficient `eta=0.1`.

!media media/tensor_mechanics/vandv/massless_small_impulse.png
      style=width:450px;margin-left:110px;float:center;
      id=fig:impulseload4
      caption=Impulse load pattern applied at the free end of the beam.

!listing dyn_euler_small_added_mass_inertia_damping.i

### Results

 The displacement, velocity and acceleration, as the function of time, at the free end of the cantilever beam in MOOSE, are compared with the results from ABAQUS using the time step of 0.1 s, as shown in [fig:mass_damping_disp], [fig:mass_damping_vel] and [fig:mass_damping_acc].

!media media/tensor_mechanics/vandv/mass_damping_disp.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:mass_damping_disp
      caption=Displacement at the free end of the massless beam with lumped mass having rotational moment of inertia.

!media media/tensor_mechanics/vandv/mass_damping_vel.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:mass_damping_vel
      caption=Velocity at the free end of the massless beam with lumped mass having rotational moment of inertia.

!media media/tensor_mechanics/vandv/mass_damping_acc.png
      style=width:500px;margin-left:110px;float:center;
      id=fig:mass_damping_acc
      caption=Acceleration at the free end of the massless beam with lumped mass having rotational moment of inertia.

!bibtex bibliography
