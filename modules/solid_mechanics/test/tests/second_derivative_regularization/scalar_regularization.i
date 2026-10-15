reg_kernel = SecondDerivativeRegularization
regularization_type = huhu

[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD9
  nx = 2
  ny = 2
[]

[Variables]
  [u]
    order = SECOND
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
  [regularization]
    type = ${reg_kernel}
    variable = u
    regularization = ${regularization_type}
    coefficient = 0.01
  []
  [source]
    type = BodyForce
    variable = u
  []
[]

[BCs]
  [zero]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Postprocessors]
  [integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
