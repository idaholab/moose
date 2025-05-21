# This test is exactly the same as 'evanescent_wave/evanescent_wave.i'
# except it uses ADMatWaveReaction and WaveEquationCoefficient instead of VectorFunctionReaction

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = waveguide_discontinuous.msh
  []
[]

[Functions]
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

  [eps_real_func]
    type = ParsedFunction
    expression = '1'
  []
  [mu_real_func]
    type = ParsedFunction
    expression = '(1 / 3e8)^2'
  []
  [k_real_func]
    type = ParsedFunction
    expression = '2*pi*20e9'
  []
[]

[Materials]
  [WaveCoeff]
    type = WaveEquationCoefficient
    eps_rel_imag = 0
    eps_rel_real = eps_real
    k_real = k_real
    mu_rel_imag = 0
    mu_rel_real = mu_real
  []
  [eps_real]
    type = ADGenericFunctionMaterial
    prop_names = eps_real
    prop_values = eps_real_func
  []
  [mu_real]
    type = ADGenericFunctionMaterial
    prop_names = mu_real
    prop_values = mu_real_func
  []
  [k_real]
    type = ADGenericFunctionMaterial
    prop_names = k_real
    prop_values = k_real_func
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
    type = ADMatWaveReaction
    variable = E_real
    field_real =  E_real
    field_imag =  E_imag
    wave_coef_real = wave_equation_coefficient_real
    wave_coef_imag = wave_equation_coefficient_imaginary
    component = real
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
    type = ADMatWaveReaction
    variable = E_imag
    field_real =  E_real
    field_imag =  E_imag
    wave_coef_real = wave_equation_coefficient_real
    wave_coef_imag = wave_equation_coefficient_imaginary
    component = imaginary
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
  file_base = 'evanescent_wave_out'
  print_linear_residuals = true
[]

[Debug]
  show_var_residual_norms = true
[]
