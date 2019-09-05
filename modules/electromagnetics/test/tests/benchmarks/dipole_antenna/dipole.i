# Verification Benchmark - Half-wave Dipole Antenna (Frequency Domain)
# Resonant Frequency = 1 GHz
# Wave Propagation Medium: Vacuum

[Mesh]
  type = FileMesh
  file = dipole_antenna_1G.msh
[]

[Variables]
  [./E_real]
    order = FIRST
    family = NEDELEC_ONE
  [../]
  [./E_imag]
    order = FIRST
    family = NEDELEC_ONE
  [../]
[]

[Kernels]
  [./curl_curl_real]
    type = CurlCurlField
    variable = E_real
  [../]
  [./coeff_real]
    type = VectorCoeffField
    variable = E_real
    func = '-(2*pi*1e9/3e8)*(2*pi*1e9/3e8)'  # -(wave number)^2
  [../]
  [./curl_curl_imag]
    type = CurlCurlField
    variable = E_imag
  [../]
  [./coeff_imag]
    type = VectorCoeffField
    variable = E_imag
    func = '-(2*pi*1e9/3e8)*(2*pi*1e9/3e8)'  # -(wave number)^2
  [../]
[]

[BCs]
  [./antenna_real]                #TODO: Replace with proper antenna surface current condition
    type = VectorCurlPenaltyDirichletBC
    penalty = 1e5
    y_exact_soln = '1'
    boundary = antenna
    variable = E_real
  [../]
  [./antenna_imag]
    type = VectorCurlPenaltyDirichletBC
    penalty = 1e5
    y_exact_soln = '1'
    boundary = antenna
    variable = E_imag
  [../]
  [./radiation_condition_real]    # Port BC without incoming wave is first order radiation condition
    type = VectorPortBC
    variable = E_real
    coupled_field = E_imag
    boundary = boundary
    component = real
    beta = 20.9439510239  # wave number at 1 GHz
  [../]
  [./radiation_condition_imag]
    type = VectorPortBC
    variable = E_imag
    coupled_field = E_real
    boundary = boundary
    component = imaginary
    beta = 20.9439510239  # wave number at 1 GHz
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
