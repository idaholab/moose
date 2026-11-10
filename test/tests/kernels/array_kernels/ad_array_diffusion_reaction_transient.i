# AD version of array_diffusion_reaction_transient.i with unit diffusion and time derivative
# coefficients; it reproduces the same gold file.
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
    type = ADArrayCoupledTimeDerivatives
    variable = u
  []
  [diff]
    type = ADArrayDiffusion
    variable = u
  []
  [reaction]
    type = ADArrayReaction
    variable = u
    reaction_coefficient = rc
  []
[]

[BCs]
  [left]
    type = ADArrayDirichletBC
    variable = u
    boundary = 1
    values = '0 0'
  []

  [right]
    type = ADArrayDirichletBC
    variable = u
    boundary = 2
    values = '1 2'
  []
[]

[Materials]
  [rc]
    type = ADGenericConstant2DArray
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
  file_base = array_diffusion_reaction_transient_out
  exodus = true
[]
