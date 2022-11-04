[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
  # This test can only be run with renumering disabled, so the
  # NodalVariableValue postprocessor's node id is well-defined.
  allow_renumbering = false
[]

[Variables]
  active = 'v'

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  active = 'v1'

  [./v1]
    order = FIRST
    family = LAGRANGE
  [../]
[]


[Functions]
  active = 'left_bc'

  [./left_bc]
    type = ParsedFunction
    expression = t
  [../]
[]

[Kernels]
  active = 'time_v diff_v'

  [./time_v]
    type = TimeDerivative
    variable = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[AuxKernels]
  active = 'ak1'

  [./ak1]
    type = CoupledAux
    variable = v1
    coupled = v
    value = 1
    operator = '+'
  [../]
[]

[BCs]
  active = 'left_v right_v'

  [./left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = '3'
    function = left_bc
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = '1'
    value = 1
  [../]
[]

[Postprocessors]
  active = 'node4v node4v1'

  [./node4v]
    type = NodalVariableValue
    variable = v
    nodeid = 3
  [../]

  [./node4v1]
    type = NodalVariableValue
    variable = v1
    nodeid = 3
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  dt = 0.1
  start_time = 0
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_nodal_aux_var_value
  exodus = true
[]
