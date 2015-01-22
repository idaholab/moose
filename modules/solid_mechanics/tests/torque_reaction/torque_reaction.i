#The calculated torque converges approximately to the theoretical value
#T = phi * G * J / L
#for a refined mesh and small angular deflection phi.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
[]

[Mesh]
  file = cylinder.e
  displacements = 'disp_x disp_y disp_z'
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]


[AuxVariables]
  [./stress_xx]      # stress aux variables are defined for output; this is a way to get integration point variables to the output file
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./saved_z]
  [../]
[]


[Functions]
  [./rampConstantAngle]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 0.1
  [../]
[]

[SolidMechanics]
  [./solid]
    save_in_disp_x = saved_x
    save_in_disp_y = saved_y
    save_in_disp_z = saved_z
  [../]
[]

[AuxKernels]
  [./stress_xx]               # computes stress components for output
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep     # for efficiency, only compute at the end of a timestep
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    execute_on = timestep_end
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep_end
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
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

  [./DisplacementAboutAxis]
    [./top]
      boundary = 2
      function = rampConstantAngle
      angle_units = degrees
      axis_origin = '10 10 10'
      axis_direction = '0 -0.70710678 0.70710678'
    [../]
  [../]

[] # BCs

[Materials]
  [./stiffStuff]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
[]


[Executioner]

  type = Transient
  # Two sets of linesearch options are for petsc 3.1 and 3.3 respectively

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'


  line_search = 'none'

  l_max_its = 50
  nl_max_its = 20
  nl_abs_tol = 1e-7
  l_tol = 1e-2

  start_time = 0.0
  dt = 1

  end_time = 1
  num_steps = 1

[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]

  [./torque]
    type = TorqueReaction
    boundary = 2
    react_x = saved_x
    react_y = saved_y
    react_z = saved_z
    axis_origin = '10 10 10'
    axis_direction = '0 -0.70710678 0.70710678'
  [../]

[]


[Outputs]
  file_base = torque_reaction_out
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
