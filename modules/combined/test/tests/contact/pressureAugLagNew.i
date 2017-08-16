[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = pressure.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./saved_z]
  [../]
  [./resid_x]
  [../]
  [./resid_y]
  [../]
  [./resid_z]
  [../]
  [./contact_traction]
  [../]
[]

[SolidMechanics]
  [./solid]
  [../]
[]

[Contact]
  [./m20_s10]
    master = 20
    slave = 10
    penalty = 1e7
    system = Constraint
    formulation = augmented_lagrange
    tangential_tolerance = 1e-6
#    penetration_tolerance = 1e-2
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 3
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]

  [./z]
    type = DirichletBC
    variable = disp_z
    boundary = 5
    value = 0.0
  [../]

  [./Pressure]
    [./press]
      boundary = 7
      factor = 1e3
    [../]
  [../]

  [./down]
    type = PresetBC
    variable = disp_y
    boundary = 8
    value = -2e-3
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1

    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff2]
    type = Elastic
    block = 2

    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
[]

[Dampers]
  [./limitX]
    type = MaxIncrement
    max_increment = 1e-5
    variable = disp_x
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  #petsc_options_iname = '-pc_type -pc_hypre_type -snes_type -snes_ls -snes_linesearch_type -ksp_gmres_restart'
  #petsc_options_value = 'hypre    boomeramg      ls         basic    basic                    101'
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'

  line_search = 'none'

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-7

  l_tol = 1e-8

  l_max_its = 100
  nl_max_its = 20
  dt = 1.0
  num_steps = 2
[]

[Outputs]
  exodus = true
[]


[Problem]
  type = ContactAugLagMulProblem
# type = ReferenceResidualProblem
  master = '2'
  slave = '3'
  penalty = 1e+7
  normalize_penalty = true
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
  contact_lagmul_tolerance_factor = 1.0
  solution_variables = 'disp_x disp_y disp_z'
  reference_residual_variables = 'saved_x saved_y saved_z'
  contact_reference_residual_variables = 'saved_x saved_y saved_z'
[]
