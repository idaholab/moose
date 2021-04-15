[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = ADDiffusion
    variable = u
  []
  [convection]
    type = ADCoupledVectorConvection
    variable = u
    velocity_vector = '0 1'
  []
[]

[BCs]
  [left]
    type = ADFunctionDirichletBC
    variable = u
    function = 1
    boundary = 'left'
  []
  [right]
    type = ADFunctionDirichletBC
    variable = u
    function = 2
    boundary = 'bottom'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 10
  dt = 0.1
[]

[Outputs]
  execute_on = TIMESTEP_END
  exodus = true
[]
