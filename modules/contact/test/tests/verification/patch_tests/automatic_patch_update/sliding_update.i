[Mesh]
  file = sliding_update.e
  displacements = 'disp_x disp_y'
  patch_size = 5
  patch_update_strategy = 'iteration'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Contact]
  [./leftright]
    secondary = 3
    primary = 2
    normalize_penalty = true
    tangential_tolerance = 1e-3
    penalty = 1e+4
    model = frictionless
    formulation = penalty
  [../]
[]

[Executioner]
   type = Transient
   solve_type = 'PJFNK'
   start_time = 0
   end_time = 10.0
   l_tol = 1e-8
   nl_rel_tol = 1e-6
   nl_abs_tol = 1e-4
   dt = 2.0
   line_search = 'none'
   petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
   petsc_options_value = 'lu superlu_dist'
   timestep_tolerance = 1e-1
[]

[BCs]
  [./fixed_1_2x]
    type = DirichletBC
    boundary = '1'
    value = 0.0
    variable = disp_x
  [../]
  [./fixed_1_2y]
    type = DirichletBC
    boundary = '1'
    value = 0.0
    variable = disp_y
  [../]
  [./sliding_1]
    type = FunctionDirichletBC
    function = sliding_fn
    variable = disp_x
    boundary = '4'
  [../]
  [./normal_y]
    type = DirichletBC
    variable = disp_y
    boundary = '4'
    value = -0.01
  [../]
#  [./Pressure]
#    [./normal_pressure]
#      disp_x = disp_x
#      disp_y = disp_y
#      factor = 100.0
#      boundary = 4
#    [../]
#  [../]
[]

[Functions]
  [./sliding_fn]
    type = ParsedFunction
    expression = 't'
  [../]
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
