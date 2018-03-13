# Test for damped small strain euler beam vibration in y direction

# An impulse load is applied at the end of a cantilever beam of length 4m.
# The properties of the cantilever beam are as follows:
# Young's modulus (E) = 1e4
# Shear modulus (G) = 4e7
# Shear coefficient (k) = 1.0
# Cross-section area (A) = 0.01
# Iy = 1e-4 = Iz
# Length (L)= 4 m
# density (rho) = 1.0
# mass proportional rayleigh damping(eta) = 0.1
# stiffness proportional rayleigh damping(eta) = 0.1
# HHT time integration parameter (alpha) = -0.3
# Corresponding Newmark beta time integration parameters beta = 0.4225 and gamma = 0.8

# For this beam, the dimensionless parameter alpha = kAGL^2/EI = 6.4e6
# Therefore, the behaves like a Euler-Bernoulli beam.

# The displacement time history from this analysis matches with that obtained from Abaqus.

[Mesh]
  type = FileMesh
  file = beam_paper_10.e
  displacements = 'disp_x disp_y disp_z'
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
    x = '0.0 0.2 0.4 10.0'
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
#petsc_options = '-snes_ksp_ew'
# petsc_options_iname = '_ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
# petsc_options_value = '201                hypre     boomeramg     4'
  l_tol = 1e-8
#  l_max_its = 50
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  start_time = 0.0
  dt = 0.2
  end_time = 5.0
  timestep_tolerance = 1e-6
[]

[Modules/LineElement]
  [./all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'

    # Material parameters
    youngs_modulus = 1e4
    shear_modulus = 4e7
    shear_coefficient = 1.0

    # Geometry parameters
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    y_orientation = '0.0 1.0 0.0'

    # dynamic simulation using consistent mass/inertia matrix
    dynamic_consistent_inertia = true

    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rot_velocities = 'rot_vel_x rot_vel_y rot_vel_z'
    rot_accelerations = 'rot_accel_x rot_accel_y rot_accel_z'

    density = 1.0
    beta = 0.4225 # Newmark time integraion parameter
    gamma = 0.8 # Newmark time integraion parameter

    # optional parameters for numerical (alpha) and Rayleigh damping
    alpha = -0.3 # HHT time integration parameter
    eta = 0.1 # Mass proportional Rayleigh damping
    zeta = 0.1 # Stiffness proportional Rayleigh damping
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
  file_base = 'dyn_euler_small_rayleigh_hht_out'
  exodus = true
  csv = true
#  [./console]
#    type = Console
#    interval = 100
#  [../]
  print_perf_log = true
[]
