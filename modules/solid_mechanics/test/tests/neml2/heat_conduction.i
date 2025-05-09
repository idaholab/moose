[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = '${units 0 m}'
  xmax = '${units 10 m}'
[]
[NEML2]
  input = 'models/libtorch_model.i'
  [thermal_conductivity_model]
    model = 'rom'
    moose_input_types = 'VARIABLE'
    moose_inputs = 'T'
    neml2_inputs = 'forces/T'
    neml2_outputs = 'state/k_T'
    moose_outputs = 'k_T'
    moose_output_types = 'MATERIAL'

    neml2_derivatives = 'state/k_T forces/T'
    moose_derivatives = 'dk_T/dT'
    moose_derivative_types = 'MATERIAL'

    export_outputs = 'k_T dk_T/dT'
    export_output_targets = 'exodus;exodus'
  []
[]
[Variables]
  [T]
    initial_condition = '${units 300 K}'
  []
[]

[BCs]
  [constant_temp_left]
    type = FunctionDirichletBC
    function = 'ramping_T'
    boundary = 'left'
    variable = T
  []
  [constant_temp_right]
    type = DirichletBC
    boundary = 'right'
    value = '${units 300 K}'
    variable = T
  []
[]
[Functions]
  [ramping_T]
    type = ParsedFunction
    expression = '300 + 1200*t/5' # Ramp from 300 K -> 1500 K over 5 seconds
  []
[]

[Kernels]
  [dT_dt]
    type = TimeDerivative
    variable = T
  []
  [head_conduction]
    type = MatDiffusion
    variable = T
    diffusivity = 'k_T'
  []
[]

[Executioner]
  type = Transient
  scheme = BDF2
  solve_type = NEWTON
  num_steps = 50
  dt = 0.1
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
