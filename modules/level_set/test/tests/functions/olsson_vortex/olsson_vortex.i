[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./vel_x]
  [../]
  [./vel_y]
  [../]
[]

[AuxKernels]
  [./vel_x_aux]
    type = FunctionAux
    variable = vel_x
    function = vel_x_func
    execute_on = 'initial timestep_end'
  [../]
  [./vel_y_aux]
    type = FunctionAux
    variable = vel_y
    function = vel_y_func
    execute_on = 'initial timestep_end'
  [../]
[]

[Functions]
  [./vel_x_func]
    type = LevelSetOlssonVortex
    component = x
  [../]
  [./vel_y_func]
    type = LevelSetOlssonVortex
    component = y
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 2
[]

[Outputs]
  exodus = true
[]
