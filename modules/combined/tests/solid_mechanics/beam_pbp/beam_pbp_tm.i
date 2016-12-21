[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  block = 1
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = beam_pbp.e
[]

[Functions]
  [./press]
    type = ParsedFunction
    value = '100*t*x*z*z*z'
  [../]
[]

[GlobalParams]
  volumetric_locking_correction=true
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./x]
    type = PresetBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./y]
    type = PresetBC
    variable = disp_y
    boundary = 10
    value = 0.0
  [../]

  [./z]
    type = PresetBC
    variable = disp_z
    boundary = 3
    value = 0.0
  [../]

  [./Pressure]
    [./the_pressure]
      boundary = 2
      function = press
      disp_x = disp_x
      disp_y = disp_y
      disp_z = disp_z
    [../]
  [../]
[] # BCs

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeFiniteStrain
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[] # Materials

[Preconditioning]
  [./PBP]
    type = PBP
    solve_order = 'disp_x disp_y disp_z'
    preconditioner = 'amg amg amg'
    off_diag_row = 'disp_y disp_z disp_z'
    off_diag_column = 'disp_x disp_x disp_y'
  [../]
[]

[Executioner]
  type = Transient

  solve_type = JFNK

  petsc_options = '-snew_ksp_ew'

  nl_abs_tol = 1e-8

  l_max_its = 100
  nl_max_its = 10
  dt = 1.0
  num_steps = 2
[] # Executioner

[Postprocessors]
  [./nonlnrits]
    type = NumNonlinearIterations
  [../]
[] # Postprocessors

[Outputs]
  file_base = tm_out
  exodus = true
[] # Outputs
