[GlobalParams]
  displacements = 'disp_x disp_y'
  preset = false
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2blk-gap_no1000.e
  []
[]

[Modules/TensorMechanics/Master]
  [./all]
    displacements = 'disp_x disp_y'
    strain = FINITE
    add_variables = true
    block = '1 2'
    use_automatic_differentiation = true
    scaling = 1e-4
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e5
    poissons_ratio = 0.345
    block = '1 2'
  [../]

  [./_elastic_strain]
    type = ADComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Contact]
  [mortar]
    mesh = file
    secondary = 101
    master = 100
    model = frictionless
    formulation = mortar
    c_normal = 1e4
    tangential_tolerance = .05
    normal_lm_scaling = 1e-3
  []
[]

[Functions]
  [./disp_bc]
    type = PiecewiseLinear
    x = '0 10.0'
    y = '0 -0.30'
  [../]

[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'left'
    value = 0.0
  [../]
  [./right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'right'
    function = disp_bc # '-30e-3 * t'
  [../]
  [./right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'right'
    function = 0
  [../]
[]

[Preconditioning]
  [./fmp]
    type = FDP
    full = true
    finite_difference_type = standard
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  num_steps = 10
  dt = 1
  dtmin = 1
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13

  petsc_options = '-ksp_monitor_true_residual -snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type  -snes_linesearch_type -pc_factor_shift_type -snes_max_it  -ksp_max_it'
  petsc_options_value = 'lu        basic                 NONZERO               20            30'

  [Predictor]
    type = SimplePredictor
    scale = 1
  []
[]

[Outputs]
  checkpoint = true
  exodus = true
  print_linear_residuals = false
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Debug]
  show_var_residual_norms = true
[]
