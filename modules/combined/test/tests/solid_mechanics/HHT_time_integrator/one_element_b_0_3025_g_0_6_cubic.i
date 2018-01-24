[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]
[Mesh]
  file = one_element.e
#  displacements = 'disp_x disp_y disp_z'
[]

#[Problem]
#  type = ReferenceResidualProblem
#  solution_variables = 'disp_x disp_y disp_z'
#  reference_residual_variables = 'saved_x saved_y saved_z'
#[]

[Variables]
  [./disp_x]
  [../]

  [./disp_y]
  [../]

  [./disp_z]
  [../]

[]

[AuxVariables]
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
#  [./saved_x]
#  [../]
#  [./saved_y]
#  [../]
#  [./saved_z]
#  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
#    save_in_disp_x = saved_x
#    save_in_disp_y = saved_y
#    save_in_disp_z = saved_z
  [../]
[]

[Kernels]
  [./inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.3025
    gamma = 0.6
#    save_in = saved_x
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.3025
    gamma = 0.6
#    save_in = saved_y
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    beta = 0.3025
    gamma = 0.6
#    save_in = saved_z
  [../]
[]

[AuxKernels]
  [./accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.3025
    execute_on = timestep_end
  [../]
  [./accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.3025
    execute_on = timestep_end
  [../]
  [./accel_z]
    type = NewmarkAccelAux
    variable = accel_z
    displacement = disp_z
    velocity = vel_z
    beta = 0.3025
    execute_on = timestep_end
  [../]
  [./vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.6
    execute_on = timestep_end
  [../]
  [./vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.6
    execute_on = timestep_end
  [../]
  [./vel_z]
    type = NewmarkVelAux
    variable = vel_z
    acceleration = accel_z
    gamma = 0.6
    execute_on = timestep_end
  [../]
[]


[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]

  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]

  [./top_z]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

  [./top_x]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0.0
  [../]

  [./top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = pull
  [../]

[]

[Materials]

  [./constant]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1.26e6
    poissons_ratio = .33
    thermal_expansion = 1e-5
  [../]

  [./density]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'density'
    prop_values = '0.00023832'
  [../]

[]

[Executioner]
#  type = Transient
#  #Preconditioned JFNK (default)
#  solve_type = 'PJFNK'
#  nl_rel_tol = 1e-10
#  l_tol = 1e-3
#  l_max_its = 100
#  dt = 2e-6
#  end_time = 2e-5

  type = Transient
  # PETSC options
  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'
  line_search = 'none'
  # controls for linear iterations
#  l_max_its = 80
#  l_tol = 8e-3
  # controls for nonlinear iterations
#  nl_max_its = 10
#  nl_rel_tol = 1e-4
#  nl_abs_tol = 1e-7
  # time control
  # Time steps set up to match halden data
  start_time = 0
  end_time = 1
#  num_steps = 5000
  dtmax = 0.1
  dtmin = 0.1
  # control for adaptive time steping
  [./TimeStepper]
    type = ConstantDT
    dt = 0.1
#    optimal_iterations = 12
#    linear_iteration_ratio = 100
#    time_t  = '-100 0' # direct control of time steps vs time (optional)
#    time_dt = '100  900'
  [../]

#  [./Quadrature]
#    order = THIRD
#  [../]
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x = '0.0 0.1      0.2     0.3    0.4        0.5      0.6   0.7      0.8    0.9    1.0'
    y = '0.0 0.000167 0.00133 0.0045 0.010667   0.020833 0.036 0.057167 0.0853 0.1215 0.16667'
    scale_factor = 1
#    type = PiecewiseLinear
#    data_file = wave_one_element.csv
#    format = columns
  [../]
[]

[Postprocessors]
#  [./ref_resid_x]
#    type = NodalL2Norm
#    execute_on = timestep_end
#    variable = saved_x
#  [../]
#  [./ref_resid_y]
#    type = NodalL2Norm
#    execute_on = timestep_end
#    variable = saved_y
#  [../]
#  [./ref_resid_z]
#    type = NodalL2Norm
#    execute_on = timestep_end
#    variable = saved_z
#  [../]
#  [./nonlinear_its]
#    type = NumNonlinearIterations
#  []
   [./_dt]
     type = TimestepSize
   [../]
   [./nonlinear_its]
     type = NumNonlinearIterations
#   [../]
#     [./disp_8]
#     type =
   [../]

[]

[Outputs]
  file_base = one_element_b_0_3025_g_0_6_cubic_out
  exodus = true
[]
