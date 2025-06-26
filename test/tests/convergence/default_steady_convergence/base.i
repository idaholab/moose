# Solves the IVPs
#
#   dS/dt = CS*(S1 - S)   x in (x1,x2)
#   S(0) = S0
#
#   dT/dt = CT*(T1 - T)   x in (x3,x4)
#   T(0) = T0
#
# on each node, which have the solutions
#
#   S(t) = S1 + (S0 - S1) exp(-CS t)
#   T(t) = T1 + (T0 - T1) exp(-CT t)
#
# Also define the aux variable:
#
#   U(S) = S^2

S0 = 500.0
S1 = 300.0
CS = 100.0

T0 = 500.0
T1 = 300.0
CT = 1.0

x1 = 0
x2 = 10

x3 = 11
x4 = 13

ss_tol = 1e-6

[Mesh]
  [S_meshgen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = ${x1}
    xmax = ${x2}
    nx = 1
    subdomain_ids = 0
    subdomain_name = S_mesh
  []
  [T_meshgen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = ${x3}
    xmax = ${x4}
    nx = 1
    subdomain_ids = 1
    subdomain_name = T_mesh
  []
  [combined]
    type = CombinerGenerator
    inputs = 'S_meshgen T_meshgen'
  []
[]

[Variables]
  [S]
    block = S_mesh
    initial_condition = ${S0}
  []
  [T]
    initial_condition = ${T0}
  []
[]

[FunctorMaterials]
  [S_mat]
    type = ADParsedFunctorMaterial
    expression = 'CS*(S1 - S)'
    functor_symbols = 'CS S1 S'
    functor_names = '${CS} ${S1} S'
    property_name = 'S_source'
  []
  [T_mat]
    type = ADParsedFunctorMaterial
    expression = 'CT*(T1 - T)'
    functor_symbols = 'CT T1 T'
    functor_names = '${CT} ${T1} T'
    property_name = 'T_source'
  []
[]

[Kernels]
  [S_time]
    type = TimeDerivative
    variable = S
  []
  [S_source]
    type = FunctorKernel
    variable = S
    functor = S_source
    functor_on_rhs = true
  []

  [T_time]
    type = TimeDerivative
    variable = T
  []
  [T_source]
    type = FunctorKernel
    variable = T
    functor = T_source
    functor_on_rhs = true
  []
[]

[AuxVariables]
  [U]
    block = S_mesh
  []
[]

[AuxKernels]
  [U_aux]
    type = ParsedAux
    variable = U
    expression = 'S^2'
    functor_names = 'S'
    functor_symbols = 'S'
  []
[]

[Executioner]
  type = Transient

  dt = 1.0
  end_time = 100.0
  steady_state_detection = true
  steady_state_convergence = steady_conv

  solve_type = NEWTON
  nl_max_its = 10
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_max_its = 10
  l_tol = 1e-3
[]

[Postprocessors]
  [num_time_steps]
    type = NumTimeSteps
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
  []
[]
