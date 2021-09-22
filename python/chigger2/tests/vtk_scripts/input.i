[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./u]
  [../]
[]

[AuxKernels]
  [./func]
    type = FunctionAux
    variable = u
    function = x+t
    execute_on = 'initial timestep_end'
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1
[]

[Outputs]
  [./out]
    type = Exodus
    execute_on = 'timestep_end'
    hide = dummy
  [../]
[]
