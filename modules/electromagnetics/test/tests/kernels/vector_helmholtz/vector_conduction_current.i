# Test for ADConductionCurrent
# Manufactured solution: E_real = y^2 * x_hat - x^2 * y_hat
#                        E_imag = y^2 * x_hat - x^2 * y_hat

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmin = -1
    ymin = -1
    elem_type = QUAD9
  []
[]

[Functions]
  #The exact solution for both the real and imag. component
  [exact]
    type = ParsedVectorFunction
    expression_x = 'y*y'
    expression_y = '-x*x'
  []

  #The forcing terms for the real and imag. component
  [source_real]
    type = ParsedVectorFunction
    symbol_names = 'omega_r mu_r epsilon_r sigma_r omega_i mu_i epsilon_i sigma_i'
    symbol_values = 'omega   mu   epsilon   sigma   omega   mu   epsilon   sigma'
    expression_x = '-epsilon_i*mu_i*omega_i^2*y^2 - 2*epsilon_i*mu_i*omega_i*omega_r*y^2 + epsilon_i*mu_i*omega_r^2*y^2 - epsilon_i*mu_r*omega_i^2*y^2 + 2*epsilon_i*mu_r*omega_i*omega_r*y^2 + epsilon_i*mu_r*omega_r^2*y^2 - epsilon_r*mu_i*omega_i^2*y^2 + 2*epsilon_r*mu_i*omega_i*omega_r*y^2 + epsilon_r*mu_i*omega_r^2*y^2 + epsilon_r*mu_r*omega_i^2*y^2 + 2*epsilon_r*mu_r*omega_i*omega_r*y^2 - epsilon_r*mu_r*omega_r^2*y^2 + mu_i*omega_i*sigma_i*y^2 + mu_i*omega_i*sigma_r*y^2 + mu_i*omega_r*sigma_i*y^2 - mu_i*omega_r*sigma_r*y^2 + mu_r*omega_i*sigma_i*y^2 - mu_r*omega_i*sigma_r*y^2 - mu_r*omega_r*sigma_i*y^2 - mu_r*omega_r*sigma_r*y^2 - 2'
    expression_y = 'epsilon_i*mu_i*omega_i^2*x^2 + 2*epsilon_i*mu_i*omega_i*omega_r*x^2 - epsilon_i*mu_i*omega_r^2*x^2 + epsilon_i*mu_r*omega_i^2*x^2 - 2*epsilon_i*mu_r*omega_i*omega_r*x^2 - epsilon_i*mu_r*omega_r^2*x^2 + epsilon_r*mu_i*omega_i^2*x^2 - 2*epsilon_r*mu_i*omega_i*omega_r*x^2 - epsilon_r*mu_i*omega_r^2*x^2 - epsilon_r*mu_r*omega_i^2*x^2 - 2*epsilon_r*mu_r*omega_i*omega_r*x^2 + epsilon_r*mu_r*omega_r^2*x^2 - mu_i*omega_i*sigma_i*x^2 - mu_i*omega_i*sigma_r*x^2 - mu_i*omega_r*sigma_i*x^2 + mu_i*omega_r*sigma_r*x^2 - mu_r*omega_i*sigma_i*x^2 + mu_r*omega_i*sigma_r*x^2 + mu_r*omega_r*sigma_i*x^2 + mu_r*omega_r*sigma_r*x^2 + 2'
  []
  [source_imag]
    type = ParsedVectorFunction
    symbol_names = 'omega_r mu_r epsilon_r sigma_r omega_i mu_i epsilon_i sigma_i'
    symbol_values = 'omega   mu   epsilon   sigma   omega   mu   epsilon   sigma'
    expression_x = '-epsilon_i*mu_i*omega_i^2*y^2 + 2*epsilon_i*mu_i*omega_i*omega_r*y^2 + epsilon_i*mu_i*omega_r^2*y^2 + epsilon_i*mu_r*omega_i^2*y^2 + 2*epsilon_i*mu_r*omega_i*omega_r*y^2 - epsilon_i*mu_r*omega_r^2*y^2 + epsilon_r*mu_i*omega_i^2*y^2 + 2*epsilon_r*mu_i*omega_i*omega_r*y^2 - epsilon_r*mu_i*omega_r^2*y^2 + epsilon_r*mu_r*omega_i^2*y^2 - 2*epsilon_r*mu_r*omega_i*omega_r*y^2 - epsilon_r*mu_r*omega_r^2*y^2 + mu_i*omega_i*sigma_i*y^2 - mu_i*omega_i*sigma_r*y^2 - mu_i*omega_r*sigma_i*y^2 - mu_i*omega_r*sigma_r*y^2 - mu_r*omega_i*sigma_i*y^2 - mu_r*omega_i*sigma_r*y^2 - mu_r*omega_r*sigma_i*y^2 + mu_r*omega_r*sigma_r*y^2 - 2'
    expression_y = 'epsilon_i*mu_i*omega_i^2*x^2 - 2*epsilon_i*mu_i*omega_i*omega_r*x^2 - epsilon_i*mu_i*omega_r^2*x^2 - epsilon_i*mu_r*omega_i^2*x^2 - 2*epsilon_i*mu_r*omega_i*omega_r*x^2 + epsilon_i*mu_r*omega_r^2*x^2 - epsilon_r*mu_i*omega_i^2*x^2 - 2*epsilon_r*mu_i*omega_i*omega_r*x^2 + epsilon_r*mu_i*omega_r^2*x^2 - epsilon_r*mu_r*omega_i^2*x^2 + 2*epsilon_r*mu_r*omega_i*omega_r*x^2 + epsilon_r*mu_r*omega_r^2*x^2 - mu_i*omega_i*sigma_i*x^2 + mu_i*omega_i*sigma_r*x^2 + mu_i*omega_r*sigma_i*x^2 + mu_i*omega_r*sigma_r*x^2 + mu_r*omega_i*sigma_i*x^2 + mu_r*omega_i*sigma_r*x^2 + mu_r*omega_r*sigma_i*x^2 - mu_r*omega_r*sigma_r*x^2 + 2'
  []

  #Material Coefficients
  [omega]
    type = ParsedFunction
    expression = '2.0'
  []
  [mu]
    type = ParsedFunction
    expression = '1.0'
  []
  [epsilon]
    type = ParsedFunction
    expression = '3.0'
  []
  [sigma]
    type = ParsedFunction
    expression = '4.0'
  []
[]

[Materials]
  [WaveCoeff]
    type = WaveEquationCoefficient
    eps_rel_imag = eps_imag
    eps_rel_real = eps_real
    k_real = k_real
    k_imag = k_imag
    mu_rel_imag = mu_imag
    mu_rel_real = mu_real
  []
  [eps_real]
    type = ADGenericFunctionMaterial
    prop_names = eps_real
    prop_values = epsilon
  []
  [eps_imag]
    type = ADGenericFunctionMaterial
    prop_names = eps_imag
    prop_values = epsilon
  []
  [mu_real]
    type = ADGenericFunctionMaterial
    prop_names = mu_real
    prop_values = mu
  []
  [mu_imag]
    type = ADGenericFunctionMaterial
    prop_names = mu_imag
    prop_values = mu
  []
  [k_real]
    type = ADGenericFunctionMaterial
    prop_names = k_real
    prop_values = omega
  []
  [k_imag]
    type = ADGenericFunctionMaterial
    prop_names = k_imag
    prop_values = omega
  []
  [cond_real]
    type = ADGenericFunctionMaterial
    prop_names = cond_real
    prop_values = sigma
  []
  [cond_imag]
    type = ADGenericFunctionMaterial
    prop_names = cond_imag
    prop_values = sigma
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
  [curl_curl_real]
    type = CurlCurlField
    variable = E_real
  []
  [coeff_real]
    type = ADMatWaveReaction
    variable = E_real
    E_real = E_real
    E_imag = E_imag
    wave_coef_real = wave_equation_coefficient_real
    wave_coef_imag = wave_equation_coefficient_imaginary
    component = real
  []
  [conduction_real]
    type = ADConductionCurrent
    variable = E_real
    E_imag = E_imag
    E_real = E_real
    conductivity_real = cond_real
    conductivity_imag = cond_imag
    ang_freq_real = k_real
    ang_freq_imag = k_imag
    permeability_real = mu_real
    permeability_imag = mu_imag
    component = real
  []
  [body_force_real]
    type = VectorBodyForce
    variable = E_real
    function = source_real
  []

  [curl_curl_imag]
    type = CurlCurlField
    variable = E_imag
  []
  [coeff_imag]
    type = ADMatWaveReaction
    variable = E_imag
    E_real = E_real
    E_imag = E_imag
    wave_coef_real = wave_equation_coefficient_real
    wave_coef_imag = wave_equation_coefficient_imaginary
    component = imaginary
  []
  [conduction_imag]
    type = ADConductionCurrent
    variable = E_imag
    E_imag = E_imag
    E_real = E_real
    conductivity_real = cond_real
    conductivity_imag = cond_imag
    ang_freq_real = k_real
    ang_freq_imag = k_imag
    permeability_real = mu_real
    permeability_imag = mu_imag
    component = imaginary
  []
  [body_force_imag]
    type = VectorBodyForce
    variable = E_imag
    function = source_imag
  []
[]

[BCs]
  [sides_real]
    type = VectorCurlPenaltyDirichletBC
    variable = E_real
    function_x = 'y*y'
    function_y = '-x*x'
    penalty = 1e8
    boundary = 'left right top bottom'
  []
  [sides_imag]
    type = VectorCurlPenaltyDirichletBC
    variable = E_imag
    function_x = 'y*y'
    function_y = '-x*x'
    penalty = 1e8
    boundary = 'left right top bottom'
  []
[]

[Postprocessors]
  [error_real]
    type = ElementVectorL2Error
    variable = E_real
    function = exact
  []
  [error_imag]
    type = ElementVectorL2Error
    variable = E_imag
    function = exact
  []

  [h]
    type = AverageElementSize
  []
  [h_squared]
    type = ParsedPostprocessor
    pp_names = 'h'
    expression = 'h * h'
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
  csv = true
[]
