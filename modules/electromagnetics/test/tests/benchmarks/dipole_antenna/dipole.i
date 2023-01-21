# Verification Benchmark - Half-wave Dipole Antenna (Frequency Domain)
# Resonant Frequency = 1 GHz
# Wave Propagation Medium: Vacuum

[Mesh]
  [file_mesh]
    type = FileMeshGenerator
    file = dipole_antenna_1G.msh
  []
  [refine]
    type = RefineBlockGenerator
    input = file_mesh
    block = 'vacuum'
    refinement = 2
  []
[]

[Variables]
  [E_real]
    order = FIRST
    family = NEDELEC_ONE
  []
  [E_imag]
    order = FIRST
    family = NEDELEC_ONE
  []
[]

[Functions]
  [WaveNumberSquared]
    type = ParsedFunction
    expression = '(2*pi*1e9/3e8)*(2*pi*1e9/3e8)'
  []
[]

[Kernels]
  [curl_curl_real]
    type = CurlCurlField
    variable = E_real
  []
  [coeff_real]
    type = VectorFunctionReaction
    variable = E_real
    function = WaveNumberSquared
    sign = negative
  []
  [curl_curl_imag]
    type = CurlCurlField
    variable = E_imag
  []
  [coeff_imag]
    type = VectorFunctionReaction
    variable = E_imag
    function = WaveNumberSquared
    sign = negative
  []
[]

[BCs]
  [antenna_real]                          # Impose exact solution of E-field onto antenna surface.
    type = VectorCurlPenaltyDirichletBC   # Replace with proper antenna surface current condition.
    penalty = 1e5
    function_y = '1'
    boundary = antenna
    variable = E_real
  []
  [antenna_imag]
    type = VectorCurlPenaltyDirichletBC
    penalty = 1e5
    function_y = '1'
    boundary = antenna
    variable = E_imag
  []
  [radiation_condition_real]
    type = VectorEMRobinBC
    variable = E_real
    coupled_field = E_imag
    boundary = boundary
    component = real
    mode = absorbing
    beta = 20.9439510239  # wave number at 1 GHz
  []
  [radiation_condition_imag]
    type = VectorEMRobinBC
    variable = E_imag
    coupled_field = E_real
    boundary = boundary
    component = imaginary
    mode = absorbing
    beta = 20.9439510239  # wave number at 1 GHz
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
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
