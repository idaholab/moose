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
  [v]
  []
[]

[Kernels]
  [u_diff]
    type = ${AD}MatDiffusion
    variable = u
    diffusivity = Du
  []
  [u_dt]
    type = ${AD}TimeDerivative
    variable = u
  []
  [v_diff]
    type = ${AD}MatDiffusion
    variable = v
    diffusivity = Dv
  []
  [v_dt]
    type = ${AD}TimeDerivative
    variable = v
  []
[]

[Materials]
  [Du]
    type = ${AD}DerivativeParsedMaterial
    property_name = Du
    expression = '0.01+u^2+v^2'
    coupled_variables = 'u v'
    derivative_order = 1
  []
  [Dv]
    type = ${AD}DerivativeParsedMaterial
    property_name = Dv
    expression = '0.01+u+v'
    coupled_variables = 'u v'
    derivative_order = 1
  []
[]

[BCs]
  [u_left]
    type = ${AD}DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [u_top]
    type = ${AD}NeumannBC
    variable = u
    boundary = top
    value = 1
  []
  [v_right]
    type = ${AD}DirichletBC
    variable = v
    boundary = right
    value = 0
  []
  [v_bottom]
    type = ${AD}NeumannBC
    variable = v
    boundary = bottom
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
