[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmin = -1
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD9
[]

[AuxVariables]
  [./f]
    order = SECOND
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = leg2
    [../]
  [../]
  [./g]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = leg1
    [../]
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Functions]
  [./leg1]
    type = ParsedFunction
    expression = 'x'
  [../]

  [./leg2]
    type = ParsedFunction
    expression = '0.5*(3.0*x*x-1.0)'
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  [./Quadrature]
    order = fourth
  []
[]

[Postprocessors]
  [./f_dot_g]
    type = VariableInnerProduct
    variable = f
    second_variable = g
  [../]

  [./f_dot_f]
    type = VariableInnerProduct
    variable = f
    second_variable = f
  [../]

  [./norm_f]
    type = ElementL2Norm
    variable = f
  [../]
[]

[Outputs]
  file_base = variable_inner_product
  csv = true
[]
