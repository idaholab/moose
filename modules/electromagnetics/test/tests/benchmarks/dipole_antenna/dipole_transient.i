# Verification Benchmark - Half-wave Dipole Antenna (Frequency Domain)
# Resonant Frequency = 1 GHz
# Wave Propagation Medium: Vacuum

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = dipole_antenna_1G.msh
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

[Kernels]
  [curl_curl_real]
    type = CurlCurlField
    variable = E_real
  []
  [time_derivative_real]
    type = VectorSecondTimeDerivative
    variable = E_real
    coefficient = '1/(3e8 * 3e8)' # 1/c^2 = mu_0 * eps_0
  []
  [curl_curl_imag]
    type = CurlCurlField
    variable = E_imag
  []
  [time_derivative_imag]
    type = VectorSecondTimeDerivative
    variable = E_imag
    coefficient = '1/(3e8 * 3e8)' # 1/c^2 = mu_0 * eps_0
  []
[]

[BCs]
  [antenna_real]                          # Impose exact solution of E-field onto antenna surface.
    type = VectorCurlPenaltyDirichletBC   # Replace with proper antenna surface current condition.
    penalty = 1e5
    function_y = 'cos(2*pi*1e9*t)'
    boundary = antenna
    variable = E_real
  []
  [antenna_imag]
    type = VectorCurlPenaltyDirichletBC
    penalty = 1e5
    function_y = 'sin(2*pi*1e9*t)'
    boundary = antenna
    variable = E_imag
  []
  [radiation_condition_real]              # First order absorbing boundary condition
    type = VectorTransientAbsorbingBC
    variable = E_real
    coupled_field = E_imag
    boundary = boundary
    component = real
  []
  [radiation_condition_imag]
    type = VectorTransientAbsorbingBC
    variable = E_imag
    coupled_field = E_real
    boundary = boundary
    component = imaginary
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 150
  dt = 0.5e-10
  [TimeIntegrator]
    type = NewmarkBeta
  []
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
