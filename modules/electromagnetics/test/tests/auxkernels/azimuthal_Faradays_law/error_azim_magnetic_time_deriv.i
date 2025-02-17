# Test for AzimuthMagneticTimeDerivRZ with scalar inputs
# Manufactured solution: E_real = y^2 * x_hat - x^2 * y_hat
#                        E_imag = y^2 * x_hat - x^2 * y_hat
#                        dB_theta_real / dt = -(2*y + 2*x)

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
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
  [aux_E_real_x]
    family = MONOMIAL
    order = FIRST
  []
  [aux_E_real_y]
    family = MONOMIAL
    order = FIRST
  []

  [azim_dB_dt_term]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [aux_E_real_x]
    type = VectorVariableComponentAux
    variable = aux_E_real_x
    vector_variable = E_real
    component = X
  []
  [aux_E_real_y]
    type = VectorVariableComponentAux
    variable = aux_E_real_y
    vector_variable = E_real
    component = Y
  []

  [aux_azim_dB_dt]
    type = AzimuthMagneticTimeDerivRZ
    # Efield = E_real
    # Efield_X = aux_E_real_x
    # Efield_Y = aux_E_real_y
    variable = azim_dB_dt_term
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
