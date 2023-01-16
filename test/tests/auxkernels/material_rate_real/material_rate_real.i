[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[AuxVariables]
  [rate]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [rate]
    type = MaterialRateRealAux
    variable = rate
    property = prop
  []
[]

[Variables]
  [u]
  []
[]

[Functions]
  [func]
    type = ParsedFunction
    expression = t*t/2
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = prop
    prop_values = func
    block = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 1
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Postprocessors]
  [rate]
    type = PointValue
    point = '0.5 0.5 0'
    variable = rate
  []
[]

[Outputs]
  csv = True
[]
