# Test for small strain euler beam vibration in y direction

# An impulse load is applied at the end of a cantilever beam of length 4m.
# The properties of the cantilever beam are as follows:
# Young's modulus (E) = 1e4
# Shear modulus (G) = 4e7
# Shear coefficient (k) = 1.0
# Cross-section area (A) = 0.01
# Iy = 1e-4 = Iz
# Length (L)= 4 m
# density (rho) = 1.0

# For this beam, the dimensionless parameter alpha = kAGL^2/EI = 6.4e6
# Therefore, the beam behaves like a Euler-Bernoulli beam.

# The theoretical first and third frequencies of this beam are:
# f1 = 1/(2 pi) * (3.5156/L^2) * sqrt(EI/rho)
# f2 = 6.268 f1

# This implies that the corresponding time period of this beam are 2.858 s and 0.455s

# The FEM solution for this beam with 10 element gives time periods of 2.856 s and 0.4505s with a time step of 0.01.
# A smaller time step is required to obtain a closer match for the lower time periods/higher frequencies.
# A higher time step of 0.05 is used in this test to reduce testing time.

# The time history from this analysis matches with that obtained from Abaqus.

[Mesh]
  type = FileMesh
  file = beam_paper_10.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./vel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./vel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./vel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_z]
  order = FIRST
  family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./accel_z]
    type = NewmarkAccelAux
    variable = accel_z
    displacement = disp_z
    velocity = vel_z
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_z]
    type = NewmarkVelAux
    variable = vel_z
    acceleration = accel_z
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./rot_accel_x]
    type = NewmarkAccelAux
    variable = rot_accel_x
    displacement = rot_x
    velocity = rot_vel_x
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./rot_vel_x]
    type = NewmarkVelAux
    variable = rot_vel_x
    acceleration = rot_accel_x
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./rot_accel_y]
    type = NewmarkAccelAux
    variable = rot_accel_y
    displacement = rot_y
    velocity = rot_vel_y
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./rot_vel_y]
    type = NewmarkVelAux
    variable = rot_vel_y
    acceleration = rot_accel_y
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./rot_accel_z]
    type = NewmarkAccelAux
    variable = rot_accel_z
    displacement = rot_z
    velocity = rot_vel_z
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./rot_vel_z]
    type = NewmarkVelAux
    variable = rot_vel_z
    acceleration = rot_accel_z
    gamma = 0.5
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = 1
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = 1
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = 1
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_y2]
    type = UserForcingFunctionNodalKernel
    variable = disp_y
    boundary = 2
    function = force
  [../]
[]

[Functions]
  [./force]
    type = PiecewiseLinear
    x = '0.0 0.05 0.1 10.0'
    y = '0.0 1.0  0.0  0.0'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
#  petsc_options_iname = '-pc_type -ksp_gmres_restart'
#  petsc_options_value = 'jacobi   101'
  line_search = 'none'
#  petsc_options = '-snes_check_jacobian -snes_check_jacobian_view'
# petsc_options = '-snes_check_jacobian'
# petsc_options = '-snes_ksp_ew'
# petsc_options_iname = '_ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
#  petsc_options_value = '201                hypre     boomeramg     4'
#  l_tol = 1e-8
#  l_max_its = 50
#  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  start_time = 0.0
  dt = 0.05
  end_time = 5.0
  timestep_tolerance = 1e-6
[]

[Kernels]
  [./solid_disp_x]
    type = StressDivergenceTensorsBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
  [../]
  [./solid_disp_y]
    type = StressDivergenceTensorsBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
  [../]
  [./solid_disp_z]
    type = StressDivergenceTensorsBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
  [../]
  [./solid_rot_x]
    type = StressDivergenceTensorsBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
  [../]
  [./solid_rot_y]
    type = StressDivergenceTensorsBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
  [../]
  [./solid_rot_z]
    type = StressDivergenceTensorsBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
  [../]
  [./inertial_force_x]
    type = InertialForceBeam
    block = 1
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rot_velocities = 'rot_vel_x rot_vel_y rot_vel_z'
    rot_accelerations = 'rot_accel_x rot_accel_y rot_accel_z'
    beta = 0.25
    gamma = 0.5
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 0
    variable = disp_x
  [../]
  [./inertial_force_y]
    type = InertialForceBeam
    block = 1
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rot_velocities = 'rot_vel_x rot_vel_y rot_vel_z'
    rot_accelerations = 'rot_accel_x rot_accel_y rot_accel_z'
    beta = 0.25
    gamma = 0.5
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 1
    variable = disp_y
  [../]
  [./inertial_force_z]
    type = InertialForceBeam
    block = 1
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rot_velocities = 'rot_vel_x rot_vel_y rot_vel_z'
    rot_accelerations = 'rot_accel_x rot_accel_y rot_accel_z'
    beta = 0.25
    gamma = 0.5
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 2
    variable = disp_z
  [../]
  [./inertial_force_rot_x]
    type = InertialForceBeam
    block = 1
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rot_velocities = 'rot_vel_x rot_vel_y rot_vel_z'
    rot_accelerations = 'rot_accel_x rot_accel_y rot_accel_z'
    beta = 0.25
    gamma = 0.5
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 3
    variable = rot_x
  [../]
  [./inertial_force_rot_y]
    type = InertialForceBeam
    block = 1
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rot_velocities = 'rot_vel_x rot_vel_y rot_vel_z'
    rot_accelerations = 'rot_accel_x rot_accel_y rot_accel_z'
    beta = 0.25
    gamma = 0.5
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 4
    variable = rot_y
  [../]
  [./inertial_force_rot_z]
    type = InertialForceBeam
    block = 1
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rot_velocities = 'rot_vel_x rot_vel_y rot_vel_z'
    rot_accelerations = 'rot_accel_x rot_accel_y rot_accel_z'
    beta = 0.25
    gamma = 0.5
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 5
    variable = rot_z
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 1.0e4
    shear_modulus = 4.0e7
    shear_coefficient = 1.0
    block = 1
  [../]
  [./strain]
    type = ComputeIncrementalBeamStrain
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 0.01
    Ay = 0.0
    Az = 0.0
    Iy = 1.0e-4
    Iz = 1.0e-4
    y_orientation = '0.0 1.0 0.0'
  [../]
  [./stress]
    type = ComputeBeamForces
    block = 1
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'density'
    prop_values = '1.0'
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_y]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = disp_y
  [../]
  [./vel_y]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = vel_y
  [../]
  [./accel_y]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = accel_y
  [../]
[]

[Outputs]
  exodus = true
  csv = true
#  [./console]
#    type = Console
#    interval = 100
#  [../]
  print_perf_log = true
[]
