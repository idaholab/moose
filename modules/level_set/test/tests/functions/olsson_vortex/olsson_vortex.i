[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
[]

[AuxKernels]
  [./vec]
    type = VectorFunctionAux
    variable = velocity
    function = velocity_func
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Functions]
  [./velocity_func]
    type = LevelSetOlssonVortex
    reverse_time = 2
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
