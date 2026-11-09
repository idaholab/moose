[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [a]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 2
    xmax = 1
    ymax = 0.5
    boundary_name_prefix = a
  []
  [b0]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 2
    xmin = 0.25
    xmax = 0.75
    ymin = 0.5
    ymax = 1
    boundary_name_prefix = b
    boundary_id_offset = 10
  []
  [b]
    type = SubdomainIDGenerator
    input = b0
    subdomain_id = 1
  []
  [c0]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 2
    xmin = 0.1
    xmax = 0.9
    ymin = 1
    ymax = 1.5
    boundary_name_prefix = c
    boundary_id_offset = 20
  []
  [c]
    type = SubdomainIDGenerator
    input = c0
    subdomain_id = 2
  []
  [combine]
    type = CombinerGenerator
    inputs = 'a b c'
  []
[]

[Problem]
  type = AugmentedLagrangianContactFEProblem
[]

[Physics/SolidMechanics/QuasiStatic/all]
  strain = SMALL
  add_variables = true
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[BCs]
  [b_fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = b_top
    value = 0
  []
  [a_fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = a_bottom
    value = 0
  []
  [a_fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = a_bottom
    value = 0
  []
  [c_fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = c_top
    value = 0
  []
  [c_push_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = c_top
    function = '-0.01 * t'
  []
[]

[Contact]
  [one]
    formulation = mortar_penalty
    model = frictionless
    primary = a_top
    secondary = b_bottom
    penalty = 1e3
    al_penetration_tolerance = 1e-6
  []
  [two]
    formulation = mortar_penalty
    model = frictionless
    primary = b_top
    secondary = c_bottom
    penalty = 1e3
    al_penetration_tolerance = 1e-6
  []
[]

[Postprocessors]
  [c_bottom_y]
    type = NodalExtremeValue
    variable = disp_y
    value_type = min
    boundary = c_bottom
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  line_search = none
  nl_abs_tol = 1e-9
  dt = 0.05
  end_time = 0.05
[]

[Outputs]
  csv = true
[]
