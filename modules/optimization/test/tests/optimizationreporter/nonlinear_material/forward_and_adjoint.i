[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 2
    ymax = 2
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [forwardT]
  []
  [adjointT]
    nl_sys = adjoint
  []
[]

[Kernels]
  [heat_conduction]
    type = ADMatDiffusion
    variable = forwardT
    diffusivity = 'conductivity'
  []
  [heat_source]
    type = ADBodyForce
    function = volumetric_heat_func
    variable = forwardT
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = adjointT
    x_coord_name = measurement_locations/measurement_xcoord
    y_coord_name = measurement_locations/measurement_ycoord
    z_coord_name = measurement_locations/measurement_zcoord
    value_name   = measurement_locations/misfit_values
  []
[]

[Materials]
  [NonlinearConductivity]
    type = ADParsedMaterial
    property_name = conductivity
    expression = '10+500*forwardT'
    coupled_variables = 'forwardT'
  []
[]

[Functions]
  [volumetric_heat_func]
    type = ParsedOptimizationFunction
    expression = q
    param_symbol_names = 'q'
    param_vector_name = 'params/heat_source'
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = forwardT
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = forwardT
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = forwardT
    boundary = bottom
    value = 2
  []
  [top]
    type = DirichletBC
    variable = forwardT
    boundary = top
    value = 1
  []
[]

[Reporters]
  [measurement_locations]
    type = OptimizationData
    variable = forwardT
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'heat_source'
    real_vector_values = '0' # Dummy
  []
[]

[VectorPostprocessors]
  [gradient_vpp]
    type = ElementOptimizationSourceFunctionInnerProduct
    function = volumetric_heat_func
    variable = adjointT
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  line_search = none
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  console = false
[]
