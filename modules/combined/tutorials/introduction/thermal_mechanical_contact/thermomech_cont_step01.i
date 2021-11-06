#
# A first attempt at thermo mechanical contact
# https://mooseframework.inl.gov/modules/combined/tutorials/introduction/step01.html
#

[GlobalParams]
  displacements = 'disp_x disp_y'
  block = 0
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
    nx = 6
    ny = 15
    xmin = 0.1
    xmax = 0.6
    ymax = 4.999
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

[Variables]
  # temperature field variable
  [T]
    # initialize to an average temperature
    initial_condition = 50
    order = FIRST
    family = LAGRANGE
  []
  # temperature lagrange multiplier
  [Tlm]
    block = 'pillars_secondary_subdomain'
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
  [dTdt]
    type = HeatConductionTimeDerivative
    variable = T
  []
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
    formulation = mortar
  []
[]

[Constraints]
  # thermal contact constraint
  [Tlm]
    type = GapConductanceConstraint
    variable = Tlm
    secondary_variable = T
    use_displaced_mesh = true
    k = 1e-1
    primary_boundary = pillar1_right
    primary_subdomain = pillars_primary_subdomain
    secondary_boundary = pillar2_left
    secondary_subdomain = pillars_secondary_subdomain
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
      function = 1e4*t^2
    []
  []

  # thermal boundary conditions (pillars are heated/cooled from the bottom)
  [heat_left]
    type = DirichletBC
    variable = T
    boundary = pillar1_bottom
    value = 100
  []
  [cool_right]
    type = DirichletBC
    variable = T
    boundary = pillar2_bottom
    value = 0
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

  # thermal properties
  [thermal_conductivity]
    type = HeatConductionMaterial
    thermal_conductivity = 100
    specific_heat = 1
  []
  [density]
    type = Density
    density = 1
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none
  # we deal with the saddle point structure of the system by adding a small shift
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       nonzero'
  end_time = 5
  dt = 0.1
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
