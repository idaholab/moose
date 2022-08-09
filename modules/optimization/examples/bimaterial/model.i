# Heat conduction in a 2D domain with two diffusivities
# The domain is -5 <= x <= 5 and -5 <= y <= 5
# The top-half of the domain (y > 0) has high diffusivity
# The top-half of the domain (y < 0) has low diffusivity
# Temperature is initialised to zero
# Temperature is fixed at temperature = 1 at centre of the domain (which is on the interface between the top and bottom halves)
#
# Assume experimental observations of the temperature are made at time = 1, and yield:
# at point = (-2, -2, 0), temperature = 0.022
# at point = (0, -2, 0), temperature = 0.040
# at point = (2, -2, 0), temperature = 0.022
# at point = (0, 2, 0), temperature = 0.138
# Notice that the temperature is higherin the top (y > 0) half of the domain because the diffusivity is higher there
#
# This MOOSE model will give different predictions for temperature, depending on the time-step size and the spatial resolution
# For dt = 1, nx = 100, ny = 100, the experimental observations are well-matched for
# diffusivity of top ~ 10
# diffusivity of bottom ~ 1
#
# The purpose of the main.i input file is to find this solution
#
# The unusual things about this input file are:
# - the diffusivity values are not just stored in two Materials.  Instead, the Materials get their values from Functions (they are GenericFunctionMaterials) which get their values from Postprocessors, which get their values from a VectorPostprocessor.  The reason for this rather convoluted approach is because the VectorPostprocessor can be controlled by a MultiAppReporterTransfer in main.i
# - the existance of the MeasuredDataPointSampler VectorPostprocessor: it simply records the difference between the MOOSE prediction and the experimental observations
#
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 14 # keep this even to ensure there is a central node
    ny = 14 # keep this even to ensure there is a central node
    xmin = -5
    xmax = 5
    ymin = -5
    ymax = 5
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
    old_block_id = '0 1'
    new_block_name = 'top bottom'
  []
  [central_node]
    type = ExtraNodesetGenerator
    input = name_blocks
    coord = '0 0 0'
    new_boundary = central_node
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [dot]
    type = TimeDerivative
    variable = temperature
  []
  [conduction]
    type = MatDiffusion
    diffusivity = diffusivity
    variable = temperature
  [../]
[]

[BCs]
  [central_node_fixed]
    type = DirichletBC
    variable = temperature
    boundary = central_node
    value = 1
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
    block = top
    prop_names = diffusivity
    prop_values = diffusivity_top_function
  []
  [mat_bottom]
    type = GenericFunctionMaterial
    block = bottom
    prop_names = diffusivity
    prop_values = diffusivity_bottom_function
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
  [T_top]
    type = PointValue
    point = '0 2 0'
    variable = temperature
  []
  [T_bottom_left]
    type = PointValue
    point = '-2 -2 0'
    variable = temperature
  []
  [T_bottom_mid]
    type = PointValue
    point = '0 -2 0'
    variable = temperature
  []
  [T_bottom_right]
    type = PointValue
    point = '2 -2 0'
    variable = temperature
  []
[]

[VectorPostprocessors]
  [vector_pp]
    type = ConstantVectorPostprocessor
    vector_names = diffusivity_values
    value = '1.0 10.0'
  []
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
  []
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]


[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  console = true
  print_linear_residuals = false
  exodus = false
  csv=false
[]
