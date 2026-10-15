[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [bottom]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 2
    xmax = 1
    ymax = 0.5
    boundary_name_prefix = bottom
  []
  [top]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 2
    xmin = 0.25
    xmax = 0.75
    ymin = 0.5
    ymax = 1
    boundary_name_prefix = top
    boundary_id_offset = 10
  []
  [top_block]
    type = SubdomainIDGenerator
    input = top
    subdomain_id = 1
  []
  [combine]
    type = CombinerGenerator
    inputs = 'bottom top_block'
  []
[]

[Problem]
  type = AugmentedLagrangianContactProblem
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
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom_bottom
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom_bottom
    value = 0
  []
  [top_x]
    type = DirichletBC
    variable = disp_x
    boundary = top_top
    value = 0
  []
  [top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top_top
    function = '-0.01 * t'
  []
[]

[Contact]
  [contact]
    formulation = mortar_penalty
    model = frictionless
    primary = bottom_top
    secondary = top_bottom
    penalty = 1e3
    al_penetration_tolerance = 1e-6
  []
[]

[Postprocessors]
  [num_nl]
    type = NumNonlinearIterations
  []
  [min_disp_y]
    type = NodalExtremeValue
    variable = disp_y
    value_type = min
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = bt
  num_steps = 1
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
[]
