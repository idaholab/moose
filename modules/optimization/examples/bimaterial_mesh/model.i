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
  [aux_diffusivity]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [aux_diffusivity]
    type = FunctionAux
    variable = aux_diffusivity
    function = diffusivity_function
    execute_on=TIMESTEP_END
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
    type = ParameterMeshFunction
    exodus_mesh = parameter_mesh_in.e
    family = MONOMIAL
    order = CONSTANT
    parameter_name = data/diffusivity
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
    real_vector_names = 'diffusivity'
    real_vector_values = '0'
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
  # exodus = true
[]
