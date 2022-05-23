# Test to verify the fundamental natural frequency of a one element ADComputeShellStress
# BCs: Clamped on one end, free on others.
# Initial perturbation applied to edge of the beam. After that, the shell vibrates freely.
#
# Results have been compared for various thicknesses with the following approximate Results
# (Moose results were obtained with 8 elements along the length)
# Thickness = 0.1. Reference freq: 10.785 Hz, Moose freq: 10.612 Hz
# Thickness = 0.05. Reference freq: 5.393 Hz, Moose freq: 5.335 Hz
# Thickness = 0.025. Reference freq: 2.696 Hz, Moose freq: 2.660 Hz
#
# Reference values have been obtained from Robert Blevins, "Formulas for Dynamics, Acoustics and Vibration",
# Table 5.3 case 11. Formula looks like: f = lambda^2/(2*pi*a^2) * sqrt(E*h^2/(12*(1-nu*nu))), where lambda
# changes as a function of shell dimensions.

# This test uses one single element for speed reasons.

# Here, the shell, instead of being on the XY plane, is oriented at a 45 deg. angle
# with respect to the Y axis.

[Mesh]
  type = FileMesh
  file = shell_inclined.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./rot_x]
  [../]
  [./rot_y]
  [../]
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  # aux variables for dynamics
  [./vel_x]
  [../]
  [./vel_y]
  [../]
  [./vel_z]
  [../]
  [./accel_x]
  [../]
  [./accel_y]
  [../]
  [./accel_z]
  [../]
  [./rot_vel_x]
  [../]
  [./rot_vel_y]
  [../]
  [./rot_accel_x]
  [../]
  [./rot_accel_y]
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = global_stress_t_points_0
    index_i = 1
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    variable = stress_yz
    rank_two_tensor = global_stress_t_points_0
    index_i = 1
    index_j = 2
  [../]

# Kernels for dynamics
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
[]

[BCs]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = '0'
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = '0'
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = '0'
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = '0'
    value = 0.0
  [../]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = '0'
    value = 0.0
  [../]
[]

[Functions]
  [./force_function]
    type = PiecewiseLinear
    x = '0.0 0.01 0.15 10.0'
    y = '0.0 0.01 0.0 0.0'
  [../]
[]
[NodalKernels]
  [./force_y2]
    type = UserForcingFunctionNodalKernel
    variable = disp_z
    boundary = '2'
    function = force_function
  [../]
[]
[Kernels]
  [./solid_disp_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  [../]
  [./solid_disp_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  [../]
  [./solid_disp_z]
    type = ADStressDivergenceShell
    block = '0'
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  [../]
  [./solid_rot_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  [../]
  [./solid_rot_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  [../]

  [./inertial_force_x]
    type = ADInertialForceShell
    use_displaced_mesh = true
    eta = 0.0
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rotational_velocities = 'rot_vel_x rot_vel_y'
    rotational_accelerations = 'rot_accel_x rot_accel_y'
    component = 0
    variable = disp_x
    thickness = 0.1
  [../]

  [./inertial_force_y]
    type = ADInertialForceShell
    use_displaced_mesh = true
    eta = 0.0
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rotational_velocities = 'rot_vel_x rot_vel_y'
    rotational_accelerations = 'rot_accel_x rot_accel_y'
    component = 1
    variable = disp_y
    thickness = 0.1

  [../]

  [./inertial_force_z]
    type = ADInertialForceShell
    use_displaced_mesh = true
    eta = 0.0
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rotational_velocities = 'rot_vel_x rot_vel_y'
    rotational_accelerations = 'rot_accel_x rot_accel_y'
    component = 2
    variable = disp_z
    thickness = 0.1

  [../]

  [./inertial_force_rot_x]
    type = ADInertialForceShell
    use_displaced_mesh = true
    eta = 0.0
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rotational_velocities = 'rot_vel_x rot_vel_y'
    rotational_accelerations = 'rot_accel_x rot_accel_y'
    component = 3
    variable = rot_x
    thickness = 0.1
  [../]

  [./inertial_force_rot_y]
    type = ADInertialForceShell
    use_displaced_mesh = true
    eta = 0.0
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rotational_velocities = 'rot_vel_x rot_vel_y'
    rotational_accelerations = 'rot_accel_x rot_accel_y'
    component = 4
    variable = rot_y
    thickness = 0.1
  [../]
[]

[Materials]
  [./elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 2100000
    poissons_ratio = 0.3
    block = 0
    through_thickness_order = SECOND
  [../]
  [./strain]
    type = ADComputeIncrementalShellStrain
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.1
    through_thickness_order = SECOND
  [../]
  [./stress]
    type = ADComputeShellStress
    block = 0
    through_thickness_order = SECOND
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'density'
    prop_values = '1.0'
  [../]
[]

[Postprocessors]
  [./disp_z_tip]
    type = PointValue
    point = '0.0 1.06 1.06'
    variable = disp_z
  [../]
  [./rot_x_tip]
    type = PointValue
    point = '0.0 1.06 1.06'
    variable = rot_x
  [../]
  [./stress_yy_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_yy
  [../]
  [./stress_yy_el_1]
    type = ElementalVariableValue
    elementid = 1
    variable = stress_yy
  [../]
  [./stress_yy_el_2]
    type = ElementalVariableValue
    elementid = 2
    variable = stress_yy
  [../]
  [./stress_yy_el_3]
    type = ElementalVariableValue
    elementid = 3
    variable = stress_yy
  [../]
  [./stress_yz_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_yz
  [../]
  [./stress_yz_el_1]
    type = ElementalVariableValue
    elementid = 1
    variable = stress_yz
  [../]
  [./stress_yz_el_2]
    type = ElementalVariableValue
    elementid = 2
    variable = stress_yz
  [../]
  [./stress_yz_el_3]
    type = ElementalVariableValue
    elementid = 3
    variable = stress_yz
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
  solve_type = PJFNK
  l_tol = 1e-11
  nl_max_its = 15
  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-10
  l_max_its = 20
  dt = 0.005
  dtmin = 0.005
  timestep_tolerance = 2e-13
  end_time = 0.5

  [./TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  [../]

[]

[Outputs]
  perf_graph = true
  csv = true
[]
