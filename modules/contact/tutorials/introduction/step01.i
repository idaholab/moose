#
# A first attempt at mechanical contact
# https://mooseframework.inl.gov/modules/contact/tutorials/introduction/step01.html
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [generated1]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 15
    xmin = -0.6
    xmax = -0.1
    ymax = 5
    bias_y = 0.9
    boundary_name_prefix = pillar1
  []
  [generated2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 15
    xmin = 0.1
    xmax = 0.6
    ymax = 5
    bias_y = 0.9
    boundary_name_prefix = pillar2
    boundary_id_offset = 4
  []
  [collect_meshes]
    type = MeshCollectionGenerator
    inputs = 'generated1 generated2'
  []

  patch_update_strategy = iteration
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'vonmises_stress'
  []
[]

[Contact]
  [pillars]
    primary = pillar1_right
    secondary = pillar2_left
    model = frictionless
    formulation = penalty
    penalty = 1e9
    normalize_penalty = true
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'pillar1_bottom pillar2_bottom'
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'pillar1_bottom pillar2_bottom'
    value = 0
  []
  [Pressure]
    [sides]
      boundary = 'pillar1_left pillar2_right'
      # we square time here to get a more progressive loading curve
      # (more pressure later on once contact is established)
      function = 1e4*t^2
    []
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e9
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  end_time = 5
  dt = 0.5
  [Predictor]
    type = SimplePredictor
    scale = 1
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  perf_graph = true
[]
