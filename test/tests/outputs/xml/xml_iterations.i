[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables/u]
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [time]
    type = ADTimeDerivative
    variable = u
  []
[]

[Functions/function]
  type = ParsedFunction
  expression = 2*x
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
    value = 2
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  solve_type = NEWTON
[]

[VectorPostprocessors]
  [line]
    type = LineFunctionSampler
    functions = function
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 5
    sort_by = x
    execute_on = 'LINEAR'
  []
[]

[Outputs]
  [out]
    type = XMLOutput
    execute_on = 'LINEAR NONLINEAR'
  []
[]
