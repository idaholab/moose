[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./ffn]
    type = ParsedFunction
    expression = '2 - t'
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = BodyForce
    variable = u
    function = ffn
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  [../]
[]

[Postprocessors]
  [./rsn]
    type = RelativeSolutionDifferenceNorm
    execute_on = TIMESTEP_END
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
