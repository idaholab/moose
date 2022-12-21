pi = 3.14159265359
period = 0.25
diff_coeff = 0.5
cp = 1.0

[Functions]
  [src_func]
    type = ParsedFunction
    value = "sin(${pi}/${period}*t)"
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    xmin = -0.5
    xmax = 0.5
    ny = 20
    ymin = -0.5
    ymax = 0.5
  []
  [source_domain]
    type = ParsedSubdomainMeshGenerator
    input = msh
    combinatorial_geometry = '(x<0.2 & x>-0.2) & (y<0.2 & y>-0.2)'
    block_id = 1
  []
[]

[Variables]
  [T]
    initial_condition = 1
  []
[]

[Kernels]
  [diffusion]
    type = CoefDiffusion
    variable = T
    coef = ${diff_coeff}
  []
  [source]
    type = BodyForce
    variable = T
    function = src_func
    block = 1
  []
  [anti_source]
    type = BodyForce
    variable = T
    value = 0
    block = 1
  []
  [time_deriv]
    type = CoefTimeDerivative
    Coefficient = ${cp}
    variable = T
  []
[]

[BCs]
  [neumann_rest]
    type = NeumannBC
    variable = T
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 25
  dt = 0.1
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-12
  line_search = 'none'
[]

[Postprocessors]
  [T_max]
    type = NodalExtremeValue
    variable = T
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [control_value]
    type = LibtorchControlValuePostprocessor
    control_name = src_control
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Controls]
  [src_control]
    type = LibtorchNeuralNetControl
    parameters = "Kernels/anti_source/value"
    responses = 'T_max'
  []
[]

[Reporters]
  inactive = 'nn_parameters'
  [T_reporter]
    type = AccumulateReporter
    reporters = 'T_max/value control_value/value'
    outputs = csv_out
  []
  [nn_parameters]
    type = LibtorchArtificialNeuralNetParameters
    control_name = src_control
    execute_on = FINAL
    outputs = json_out
  []
[]

[Outputs]
  [csv_out]
    type = CSV
    execute_on = FINAL
  []
  [json_out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
  []
[]
