[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '10'
    ix = '10'
  []
[]

[Variables]
  [dummy]
  []
[]

[AuxVariables]
  [Temperature]
  []
  [Temperature_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [extra]
    type = ADDiffusion
    variable = dummy
  []
[]

[BCs]
  [extra_dummy]
    type = DirichletBC
    variable = dummy
    boundary = '1'
    value = 0.0
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
  execute_on = 'timestep_end final'
[]
