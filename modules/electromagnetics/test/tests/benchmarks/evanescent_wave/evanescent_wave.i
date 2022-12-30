# Evanescent wave decay benchmark
# frequency = 20 GHz
# eps_R = 1.0
# mu_R = 1.0

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = waveguide_discontinuous.msh
  []
[]

[Functions]
  [waveNumberSquared]
    type = ParsedFunction
    expression = '(2*pi*20e9/3e8)^2'
  []
  [omegaMu]
    type = ParsedFunction
    expression = '2*pi*20e9*4*pi*1e-7'
  []
  [beta]
    type = ParsedFunction
    expression = '2*pi*20e9/3e8'
  []
  [curr_real]
    type = ParsedVectorFunction
    expression_y = 1.0
  []
  [curr_imag] # defaults to '0.0 0.0 0.0'
    type = ParsedVectorFunction
  []
[]

[Variables]
  [E_real]
    family = NEDELEC_ONE
    order = FIRST
  []
  [E_imag]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [curlCurl_real]
    type = CurlCurlField
    variable = E_real
  []
  [coeff_real]
    type = VectorFunctionReaction
    variable = E_real
    function = waveNumberSquared
    sign = negative
  []
  [source_real]
    type = VectorCurrentSource
    variable = E_real
    component = real
    source_real = curr_real
    source_imag = curr_imag
    function_coefficient = omegaMu
    block = source
  []
  [curlCurl_imag]
    type = CurlCurlField
    variable = E_imag
  []
  [coeff_imag]
    type = VectorFunctionReaction
    variable = E_imag
    function = waveNumberSquared
    sign = negative
  []
  [source_imaginary]
    type = VectorCurrentSource
    variable = E_imag
    component = imaginary
    source_real = curr_real
    source_imag = curr_imag
    function_coefficient = omegaMu
    block = source
  []
[]

[BCs]
  [absorbing_left_real]
    type = VectorEMRobinBC
    variable = E_real
    component = real
    beta = beta
    coupled_field = E_imag
    mode = absorbing
    boundary = 'port'
  []
  [absorbing_right_real]
    type = VectorEMRobinBC
    variable = E_real
    component = real
    beta = beta
    coupled_field = E_imag
    mode = absorbing
    boundary = 'exit'
  []
  [absorbing_left_imag]
    type = VectorEMRobinBC
    variable = E_imag
    component = imaginary
    beta = beta
    coupled_field = E_real
    mode = absorbing
    boundary = 'port'
  []
  [absorbing_right_imag]
    type = VectorEMRobinBC
    variable = E_imag
    component = imaginary
    beta = beta
    coupled_field = E_real
    mode = absorbing
    boundary = 'exit'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[Debug]
  show_var_residual_norms = true
[]
