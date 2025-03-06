# Test for AzimuthMagneticTimeDerivRZ with a vector input
# Manufactured solution: E_real = y^2 * x_hat - x^2 * y_hat
#                        E_imag = y^2 * x_hat - x^2 * y_hat
#                        dB_theta_real / dt = -(2*y + 2*x)

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 1
    xmin = 0
    ymax = 1
    ymin = -1
    elem_type = QUAD9
  []
  coord_type = RZ
  rz_coord_axis = Y
[]

[Functions]
  #The exact solution for the heated species and electric field real and imag. component
  [exact_real]
    type = ParsedVectorFunction
    expression_x = 'y^2'
    expression_y = '-x^2'
  []
  [exact_imag]
    type = ParsedVectorFunction
    expression_x = 'y^2'
    expression_y = '-x^2'
  []

  #The forcing terms for the heated species and electric field real and imag. component
  [source_real]
    type = ParsedVectorFunction
    expression_x = '-y^2 - cos(pi*y) - 2'
    expression_y = 'x^2 + cos(pi*x) + 4 + 2*y/x'
  []
  [source_imag]
    type = ParsedVectorFunction
    expression_x = '-y^2 + sin(pi*y) - 2'
    expression_y = 'x^2 - sin(pi*x) + 4 + 2*y/x'
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

  [azim_dB_dt_func]
    type = ParsedFunction
    expression = '-(2*y + 2*x)'
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
  [azim_dB_dt_term_vector]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [aux_azim_dB_dt_vector]
    type = AzimuthMagneticTimeDerivRZ
    Efield = E_real
    variable = azim_dB_dt_term_vector
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

  [error_azim_dB_dt_vector]
    type = ElementL2Error
    variable = azim_dB_dt_term_vector
    function = azim_dB_dt_func
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

  nl_rel_tol = 1e-16
[]

[Outputs]
  exodus = true
  csv = true
[]
