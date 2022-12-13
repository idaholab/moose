# Steady state Heat conduction in a 2D domain with two diffusivities
# The domain is -4 <= x <= 4 and -4 <= y <= 4
# The top-half of the domain (y > 0) has high diffusivity
# The bottom-half of the domain (y < 0) has low diffusivity

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
    xmin = -4
    xmax = 4
    ymin = -4
    ymax = 4
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = temperature
    diffusivity = diffusivity
  []
  [heat_source]
    type = BodyForce
    value = 100
    variable = temperature
  []
[]

[AuxVariables]
  [grad_Tx]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Ty]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Tz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
[]

[Functions]
  [diffusivity_function]
    type = NearestReporterCoordinatesFunction
    x_coord_name = data/coordx
    y_coord_name = data/coordy
    value_name = data/diffusivity
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = diffusivity
    prop_values = diffusivity_function
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
  []
  [data]
    type = ConstantReporter
    real_vector_names = 'coordx coordy diffusivity'
    real_vector_values = '0 0; -2 2; 5 10'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_forced_its = 1
  line_search = none
  nl_abs_tol = 1e-8
[]

[Outputs]
  file_base = 'forward'
  console = false
[]
