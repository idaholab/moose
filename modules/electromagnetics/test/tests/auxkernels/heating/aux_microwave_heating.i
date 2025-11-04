# Test for EMJouleHeatingHeatGeneratedAux
# Manufactured solution: E_real = cos(pi*y) * x_hat - cos(pi*x) * y_hat
#                        E_imag = sin(pi*y) * x_hat - sin(pi*x) * y_hat
#                        n = x^2*y^2
#                        heating = '0.5*sigma_r*(sin(x*pi)^2 + sin(y*pi)^2 + cos(x*pi)^2 + cos(y*pi)^2)'

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
  #The exact solution for the heated species and electric field real and imag. component
  [exact_real]
    type = ParsedVectorFunction
    expression_x = 'cos(pi*y)'
    expression_y = '-cos(pi*x)'
  []
  [exact_imag]
    type = ParsedVectorFunction
    expression_x = 'sin(pi*y)'
    expression_y = '-sin(pi*x)'
  []
  [exact_n]
    type = ParsedFunction
    expression = 'x^2*y^2'
  []

  #The forcing terms for the heated species and electric field real and imag. component
  [source_real]
    type = ParsedVectorFunction
    symbol_names = 'omega_r mu_r epsilon_r sigma_r omega_i mu_i epsilon_i sigma_i'
    symbol_values = 'omega   mu   epsilon   sigma   omega   mu   epsilon   sigma'
    expression_x = '-epsilon_i*mu_i*omega_i^2*cos(pi*y) - 2*epsilon_i*mu_i*omega_i*omega_r*sin(pi*y) + epsilon_i*mu_i*omega_r^2*cos(pi*y) - epsilon_i*mu_r*omega_i^2*sin(pi*y) + 2*epsilon_i*mu_r*omega_i*omega_r*cos(pi*y) + epsilon_i*mu_r*omega_r^2*sin(pi*y) - epsilon_r*mu_i*omega_i^2*sin(pi*y) + 2*epsilon_r*mu_i*omega_i*omega_r*cos(pi*y) + epsilon_r*mu_i*omega_r^2*sin(pi*y) + epsilon_r*mu_r*omega_i^2*cos(pi*y) + 2*epsilon_r*mu_r*omega_i*omega_r*sin(pi*y) - epsilon_r*mu_r*omega_r^2*cos(pi*y) + mu_i*omega_i*sigma_i*cos(pi*y) + mu_i*omega_i*sigma_r*sin(pi*y) + mu_i*omega_r*sigma_i*sin(pi*y) - mu_i*omega_r*sigma_r*cos(pi*y) + mu_r*omega_i*sigma_i*sin(pi*y) - mu_r*omega_i*sigma_r*cos(pi*y) - mu_r*omega_r*sigma_i*cos(pi*y) - mu_r*omega_r*sigma_r*sin(pi*y) + pi^2*cos(pi*y)'
    expression_y = 'epsilon_i*mu_i*omega_i^2*cos(pi*x) + 2*epsilon_i*mu_i*omega_i*omega_r*sin(pi*x) - epsilon_i*mu_i*omega_r^2*cos(pi*x) + epsilon_i*mu_r*omega_i^2*sin(pi*x) - 2*epsilon_i*mu_r*omega_i*omega_r*cos(pi*x) - epsilon_i*mu_r*omega_r^2*sin(pi*x) + epsilon_r*mu_i*omega_i^2*sin(pi*x) - 2*epsilon_r*mu_i*omega_i*omega_r*cos(pi*x) - epsilon_r*mu_i*omega_r^2*sin(pi*x) - epsilon_r*mu_r*omega_i^2*cos(pi*x) - 2*epsilon_r*mu_r*omega_i*omega_r*sin(pi*x) + epsilon_r*mu_r*omega_r^2*cos(pi*x) - mu_i*omega_i*sigma_i*cos(pi*x) - mu_i*omega_i*sigma_r*sin(pi*x) - mu_i*omega_r*sigma_i*sin(pi*x) + mu_i*omega_r*sigma_r*cos(pi*x) - mu_r*omega_i*sigma_i*sin(pi*x) + mu_r*omega_i*sigma_r*cos(pi*x) + mu_r*omega_r*sigma_i*cos(pi*x) + mu_r*omega_r*sigma_r*sin(pi*x) - pi^2*cos(pi*x)'
  []
  [source_imag]
    type = ParsedVectorFunction
    symbol_names = 'omega_r mu_r epsilon_r sigma_r omega_i mu_i epsilon_i sigma_i'
    symbol_values = 'omega   mu   epsilon   sigma   omega   mu   epsilon   sigma'
    expression_x = '-epsilon_i*mu_i*omega_i^2*sin(pi*y) + 2*epsilon_i*mu_i*omega_i*omega_r*cos(pi*y) + epsilon_i*mu_i*omega_r^2*sin(pi*y) + epsilon_i*mu_r*omega_i^2*cos(pi*y) + 2*epsilon_i*mu_r*omega_i*omega_r*sin(pi*y) - epsilon_i*mu_r*omega_r^2*cos(pi*y) + epsilon_r*mu_i*omega_i^2*cos(pi*y) + 2*epsilon_r*mu_i*omega_i*omega_r*sin(pi*y) - epsilon_r*mu_i*omega_r^2*cos(pi*y) + epsilon_r*mu_r*omega_i^2*sin(pi*y) - 2*epsilon_r*mu_r*omega_i*omega_r*cos(pi*y) - epsilon_r*mu_r*omega_r^2*sin(pi*y) + mu_i*omega_i*sigma_i*sin(pi*y) - mu_i*omega_i*sigma_r*cos(pi*y) - mu_i*omega_r*sigma_i*cos(pi*y) - mu_i*omega_r*sigma_r*sin(pi*y) - mu_r*omega_i*sigma_i*cos(pi*y) - mu_r*omega_i*sigma_r*sin(pi*y) - mu_r*omega_r*sigma_i*sin(pi*y) + mu_r*omega_r*sigma_r*cos(pi*y) + pi^2*sin(pi*y)'
    expression_y = 'epsilon_i*mu_i*omega_i^2*sin(pi*x) - 2*epsilon_i*mu_i*omega_i*omega_r*cos(pi*x) - epsilon_i*mu_i*omega_r^2*sin(pi*x) - epsilon_i*mu_r*omega_i^2*cos(pi*x) - 2*epsilon_i*mu_r*omega_i*omega_r*sin(pi*x) + epsilon_i*mu_r*omega_r^2*cos(pi*x) - epsilon_r*mu_i*omega_i^2*cos(pi*x) - 2*epsilon_r*mu_i*omega_i*omega_r*sin(pi*x) + epsilon_r*mu_i*omega_r^2*cos(pi*x) - epsilon_r*mu_r*omega_i^2*sin(pi*x) + 2*epsilon_r*mu_r*omega_i*omega_r*cos(pi*x) + epsilon_r*mu_r*omega_r^2*sin(pi*x) - mu_i*omega_i*sigma_i*sin(pi*x) + mu_i*omega_i*sigma_r*cos(pi*x) + mu_i*omega_r*sigma_i*cos(pi*x) + mu_i*omega_r*sigma_r*sin(pi*x) + mu_r*omega_i*sigma_i*cos(pi*x) + mu_r*omega_i*sigma_r*sin(pi*x) + mu_r*omega_r*sigma_i*sin(pi*x) - mu_r*omega_r*sigma_r*cos(pi*x) - pi^2*sin(pi*x)'
  []
  [source_n]
    type = ParsedFunction
    symbol_names = 'sigma_r'
    symbol_values = 'sigma'
    expression = '-2*x^2 - 2*y^2 - 0.5*sigma_r*(sin(x*pi)^2 + sin(y*pi)^2 + cos(x*pi)^2 + cos(y*pi)^2)'
  []

  [heating_func]
    type = ParsedFunction
    symbol_names = 'sigma_r'
    symbol_values = 'sigma'
    expression = '0.5*sigma_r*(sin(x*pi)^2 + sin(y*pi)^2 + cos(x*pi)^2 + cos(y*pi)^2)'
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
    #expression = 'x^2*y^2'
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
  [n]
    family = LAGRANGE
    order = FIRST
  []

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
    field_real =  E_real
    field_imag =  E_imag
    wave_coef_real = wave_equation_coefficient_real
    wave_coef_imag = wave_equation_coefficient_imaginary
    component = real
  []
  [conduction_real]
    type = ADConductionCurrent
    variable = E_real
    field_imag =  E_imag
    field_real =  E_real
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
    field_real =  E_real
    field_imag =  E_imag
    wave_coef_real = wave_equation_coefficient_real
    wave_coef_imag = wave_equation_coefficient_imaginary
    component = imaginary
  []
  [conduction_imag]
    type = ADConductionCurrent
    variable = E_imag
    field_imag =  E_imag
    field_real =  E_real
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

  [n_diffusion]
    type = Diffusion
    variable = n
  []
  [microwave_heating]
    type = EMJouleHeatingSource
    variable = n
    E_imag = E_imag
    E_real = E_real
    conductivity = cond_real
  []
  [body_force_n]
    type = BodyForce
    variable = n
    function = source_n
  []
[]

[AuxVariables]
  [heating_term]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [aux_microwave_heating]
    type = EMJouleHeatingHeatGeneratedAux
    variable = heating_term
    E_imag = E_imag
    E_real = E_real
    conductivity = cond_real
  []
[]

[BCs]
  [sides_real]
    type = VectorCurlPenaltyDirichletBC
    variable = E_real
    function = exact_real
    penalty = 1e8
    boundary = 'left right top bottom'
  []
  [sides_imag]
    type = VectorCurlPenaltyDirichletBC
    variable = E_imag
    function = exact_imag
    penalty = 1e8
    boundary = 'left right top bottom'
  []

  [sides_n]
    type = FunctorDirichletBC
    variable = n
    boundary = 'left right top bottom'
    functor = exact_n
    preset = false
  []
[]

[Postprocessors]
  [error_real]
    type = ElementVectorL2Error
    variable = E_real
    function = exact_real
  []
  [error_imag]
    type = ElementVectorL2Error
    variable = E_imag
    function = exact_imag
  []

  [error_n]
    type = ElementL2Error
    variable = n
    function = exact_n
  []

  [error_aux_heating]
    type = ElementL2Error
    variable = heating_term
    function = heating_func
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

  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]
