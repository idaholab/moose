[GlobalParams]
  order = SECOND
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = normalized_penalty_Q8.e
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Functions]
  [./left_x]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 0.02 0'
  [../]
[]
[AuxVariables]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_xx'
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
  []
[]

[Contact]
  [./m3_s2]
    primary = 3
    secondary = 2
    penalty = 1e10
    normalize_penalty = true
    formulation = penalty
    tangential_tolerance = 1e-3
  [../]
[]

[BCs]
  [./left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 1
    function = left_x
  [../]

  [./y]
    type = DirichletBC
    variable = disp_y
    boundary = '1 2 3 4'
    value = 0.0
  [../]

  [./right]
    type = DirichletBC
    variable = disp_x
    boundary = '3 4'
    value = 0
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = ComputeIsotropicElasticityTensor
    block = '1 2 3 4 1000'
    youngs_modulus = 3e8
    poissons_ratio = 0.0
  [../]
  [./stiffStuff1_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2 3 4 1000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'

  line_search = 'none'

  nl_rel_tol = 1e-12
  nl_abs_tol = 5e-8

  l_max_its = 100
  nl_max_its = 10
  dt = 0.5
  num_steps = 4
[]

[Outputs]
  exodus = true
[]
