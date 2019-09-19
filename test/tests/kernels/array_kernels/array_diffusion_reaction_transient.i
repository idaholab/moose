[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
[]

[Kernels]
  [dudt]
    type = ArrayTimeDerivative
    variable = u
    time_derivative_coefficient = tc
  []
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [reaction]
    type = ArrayReaction
    variable = u
    reaction_coefficient = rc
  []
[]

[BCs]
  [left]
    type = ArrayDirichletBC
    variable = u
    boundary = 1
    values = '0 0'
  []

  [right]
    type = ArrayDirichletBC
    variable = u
    boundary = 2
    values = '1 2'
  []
[]

[Materials]
  [tc]
    type = GenericConstantArray
    prop_name = tc
    prop_value = '1 1'
  []
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
  [rc]
    type = GenericConstant2DArray
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[Postprocessors]
  [intu0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 0
  []
  [intu1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  exodus = true
[]
