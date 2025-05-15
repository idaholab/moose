AD = ''

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ${AD}MatDiffusion
    variable = u
    diffusivity = Du
  []
  [dt]
    type = ${AD}TimeDerivative
    variable = u
  []
[]

[Materials]
  [Dc]
    type = ${AD}DerivativeParsedMaterial
    property_name = Du
    expression = '0.01+u^2'
    coupled_variables = 'u'
    derivative_order = 1
  []
[]

[BCs]
  [right]
    type = ${AD}DirichletBC
    variable = u
    boundary = right
    value = 0
  []
  [top]
    type = ${AD}NeumannBC
    variable = u
    boundary = top
    value = 1
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  scheme = 'BDF2'
  dt = 1
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
