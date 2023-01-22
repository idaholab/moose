[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Functions]
  [./ic_fn]
    type = ParsedFunction
    expression = 'x * y'
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = u
    function = ic_fn
  [../]

  [./a_ic]
    type = ConstantIC
    variable = a
    value = 1
  [../]
[]

[AuxVariables]
  [./a]
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./rhs]
    type = BodyForce
    variable = u
    function = 1
  [../]
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = 'sub.i'
    positions = '
      0.25 0.25 0
      0.75 0.75 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Transfers]
  [./master_to_sub]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub
    source_variable = u
    variable = a
  [../]

  [./sub_to_master]
    type = MultiAppUserObjectTransfer
    from_multi_app = sub
    user_object = fn_uo
    variable = a
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  exodus = true
[]
