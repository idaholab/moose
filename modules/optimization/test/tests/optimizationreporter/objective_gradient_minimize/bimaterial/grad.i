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
[]


[Reporters]
  [misfit]
    type=OptimizationData
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = temperature
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
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

[AuxVariables]
  [forwardAdjoint]
  []
  [temperature_forward]
  []
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
  [grad_Tfx]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Tfy]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Tfz]
    order = CONSTANT
    family = MONOMIAL
  []
  [gradient]
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
  [grad_Tfx]
    type = VariableGradientComponent
    component = x
    variable = grad_Tfx
    gradient_variable = temperature_forward
  []
  [grad_Tfy]
    type = VariableGradientComponent
    component = y
    variable = grad_Tfy
    gradient_variable = temperature_forward
  []
  [grad_Tfz]
    type = VariableGradientComponent
    component = z
    variable = grad_Tfz
    gradient_variable = temperature_forward
  []
  [gradient]
    type = ParsedAux
    variable = gradient
    args = 'grad_Tx grad_Ty grad_Tz grad_Tfx grad_Tfy grad_Tfz'
    function = '-grad_Tx*grad_Tfx-grad_Ty*grad_Tfy-grad_Tz*grad_Tfz'#we need to include the material derivative, which can be captured when computing the flux based on the derivative of the material.
  []
  [forwardAdjoint] # I am not sure why do we need this?
    type = ParsedAux
    variable = forwardAdjoint
    args = 'temperature_forward temperature'
    function = 'temperature_forward*temperature'
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

[Materials] #same material as what was used in the forward model
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
  ############
  # we need to combine the two in one vector.
  [grad_bottom] #compute the integral of the gradient variable on the bottom block (first parameter)
    type = ElementIntegralVariablePostprocessor
    variable = gradient
    execute_on = 'final'
    block=bottom
    outputs=grad_bottom
  []
  [grad_top] #compute the integral of the gradient variable on the bottom block (second parameter)
    type = ElementIntegralVariablePostprocessor
    variable = gradient
    execute_on = 'final'
    block=top
    outputs=grad_top
  []
  ############
[]

[VectorPostprocessors]
  [vector_pp]
    type = ConstantVectorPostprocessor
    vector_names = diffusivity_values
    value = '1.0 10.0' #we need to set initial values (any values)- these will be over-written
  []
  [gradvec]
    type = VectorOfPostprocessors
    postprocessors = 'grad_bottom grad_top'
    execute_on = 'final'
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
  console = false
  file_base = 'adjoint'
[]
