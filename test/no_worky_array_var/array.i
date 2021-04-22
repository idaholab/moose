[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []

  [v]
    order = CONSTANT
    family = MONOMIAL
    components = 2
  []
[]

[AuxVariables]
  [v_aux]
    order = CONSTANT
    family = MONOMIAL
    components = 2
  []
[]

[MultiApps]
  [run_array_sub]
    type = TransientMultiApp
    input_files = 'array_sub.i'
    relaxation_factor = 0.1
    relaxed_variables = v_aux
  []
[]

[Transfers]
  [get_v_aux]
    type = MultiAppCopyTransfer
    variable = v_aux
    source_variable = v
    direction = from_multiapp
    multi_app = run_array_sub
  []
[]

[ICs]
  [v_aux_ic]
    type = ArrayConstantIC
    variable = v_aux
    value = '1 2'
  []
[]

[Kernels]
  [u_time]
    type = TimeDerivative
    variable = u
  []
  [u_diff]
    type = Diffusion
    variable = u
  []

  [v_time]
    type = ArrayTimeDerivative
    variable = v
    time_derivative_coefficient = tc
  []
  [v_reaction]
    type = ArrayCoupledForce
    variable = v
    v = u
    coef = '1 2'
  []
[]

[Materials/tc]
  type = GenericConstantArray
  prop_name = tc
  prop_value = '2 3'
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 2
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  picard_max_its = 10
[]

[Outputs]
  exodus = true
[]
