[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  ymax = 0.1
  xmax = 0.1
[]

[AuxVariables]
  [x]
  []
  [y]
  []
[]

[ICs]
  [x]
    type = FunctionIC
    function = x
    variable = x
  []
  [y]
    type = FunctionIC
    function = y
    variable = y
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [incoming_x]
    type = Receiver
    execute_on = 'TIMESTEP_BEGIN'
  []
  [incoming_y]
    type = Receiver
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

