[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables/T]
[]

[Kernels]
  [time]
    type = ADTimeDerivative
    variable = T
  []
  [diff]
    type = ADMatDiffusion
    variable = T
    diffusivity = diffusivity
  []
  [source]
    type = ADBodyForce
    variable = T
    value = 1
    function = 1
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = T
    boundary = left
    value = -10
  []
  [right]
    type = ADNeumannBC
    variable = T
    boundary = right
    value = -100
  []
[]

[Materials/constant]
  type = ADGenericConstantMaterial
  prop_names = 'diffusivity'
  prop_values = 1
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 4
  dt = 0.25
[]

[Postprocessors]
  [T_avg]
    type = AverageNodalVariableValue
    variable = T
  []
  [q_left]
    type = ADSideFluxAverage
    variable = T
    boundary = left
    diffusivity = diffusivity
  []
[]

[Controls/stochastic]
  type = ParameterReceiver
[]

[Outputs]
[]
