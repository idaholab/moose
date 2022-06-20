# Steady state Heat conduction in a 2D domain with two diffusivities
# The domain is -4 <= x <= 4 and -4 <= y <= 4
# The top-half of the domain (y > 0) has high diffusivity
# The top-half of the domain (y < 0) has low diffusivity

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
  [bimaterial]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '-100 -100 -100'
    top_right = '100 0 100'
  []
  [name_blocks]
    type = RenameBlockGenerator
    input = bimaterial
    old_block = '0 1'
    new_block = 'top bottom'
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [conduction]
    type = MatDiffusion
    diffusivity = diffusivity
    variable = temperature
  []
  [heat_source]
    type = ADMatHeatSource
    material_property = volumetric_heat
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

[AuxKernels]
  [grad_Tx]
    type = VariableGradientComponent
    component = x
    variable = grad_Tx
    gradient_variable = temperature
  []
  [grad_Ty]
    type = VariableGradientComponent
    component = y
    variable = grad_Ty
    gradient_variable = temperature
  []
  [grad_Tz]
    type = VariableGradientComponent
    component = z
    variable = grad_Tz
    gradient_variable = temperature
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
  [diffusivity_top_function]
    type = ParsedFunction
    value = alpha
    vars = alpha
    vals = d_top
  []
  [diffusivity_bottom_function]
    type = ParsedFunction
    value = alpha
    vars = alpha
    vals = d_bot
  []
[]

[Materials]
  [mat_top]
    type = GenericFunctionMaterial
    block = 'top'
    prop_names = diffusivity
    prop_values = diffusivity_top_function
  []
  [mat_bottom]
    type = GenericFunctionMaterial
    block = 'bottom'
    prop_names = diffusivity
    prop_values = diffusivity_bottom_function
  []
  [volumetric_heat]
    type = ADGenericFunctionMaterial
    prop_names = 'volumetric_heat'
    prop_values = 100
  []
[]

[Postprocessors]
  [d_bot]
    type = VectorPostprocessorComponent
    index = 0
    vectorpostprocessor = vector_pp
    vector_name = diffusivity_values
    execute_on = 'linear'
  []
  [d_top]
    type = VectorPostprocessorComponent
    index = 1
    vectorpostprocessor = vector_pp
    vector_name = diffusivity_values
    execute_on = 'linear'
  []
[]

[VectorPostprocessors]
  [vector_pp]
    type = ConstantVectorPostprocessor
    vector_names = diffusivity_values
    value = '5 10' #we need to set initial values (any values)- these will be over-written
  []
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
  []
  # [synthetic_data]
  #   type = LineValueSampler
  #   variable = 'temperature'
  #   start_point = '0 -3.99 0'
  #   end_point = '0 3.99 0'
  #   num_points = 11
  #   sort_by = id
  # []
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_forced_iters = 1
  line_search=none
  nl_abs_tol=1e-8
[]

[Outputs]
  file_base = 'forward'
  console = false
[]
