# Test for SourceCurrentHeating
# Manufactured solution: E_real = cos(pi*y) * x_hat - cos(pi*x) * y_hat
#                        E_imag = sin(pi*y) * x_hat - sin(pi*x) * y_hat
#                        heating = 'sin(x*pi)*cos(x*pi) + sin(y*pi)*cos(y*pi)'

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

  #The forcing terms for the heated species and electric field real and imag. component
  [source_real]
    type = ParsedVectorFunction
    expression_x = '-2*cos(pi*y) + pi^2*cos(pi*y)'
    expression_y = '-pi^2*cos(pi*x) + 2*cos(pi*x)'
  []
  [source_imag]
    type = ParsedVectorFunction
    expression_x = 'pi^2*sin(pi*y)'
    expression_y = '-pi^2*sin(pi*x)'
  []

  [current_real]
    type = ParsedVectorFunction
    expression_x = 'sin(pi*y)'
    expression_y = '-sin(pi*x)'
  []
  [current_imag]
    type = ParsedVectorFunction
    expression_x = 'cos(pi*y)'
    expression_y = '-cos(pi*x)'
  []

  [heating_func]
    type = ParsedFunction
    expression = '1.0*sin(x*pi)*cos(x*pi) + 1.0*sin(y*pi)*cos(y*pi)'
  []
[]

[Materials]
  [WaveCoeff]
    type = WaveEquationCoefficient
    eps_rel_real = 1.0
    eps_rel_imag = 0.0
    k_real = 1.0
    k_imag = 0.0
    mu_rel_real = 1.0
    mu_rel_imag = 0.0
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
  [current_real]
    type = VectorCurrentSource
    variable = E_real
    source_real = current_real
    source_imag = current_imag
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
  [current_imag]
    type = VectorCurrentSource
    variable = E_imag
    source_real = current_real
    source_imag = current_imag
    component = imaginary
  []
  [body_force_imag]
    type = VectorBodyForce
    variable = E_imag
    function = source_imag
  []
[]

[AuxVariables]
  [current_heating_term]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [aux_current_heating]
    type = SourceCurrentHeating
    variable = current_heating_term
    E_real = E_real
    E_imag = E_imag
    source_real = current_real
    source_imag = current_imag
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

  [error_aux_heating]
    type = ElementL2Error
    variable = current_heating_term
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
