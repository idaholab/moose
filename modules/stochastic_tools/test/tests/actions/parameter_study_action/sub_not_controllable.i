[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [average]
    type = AverageNodalVariableValue
    variable = u
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

# This is for testing distributions
p0 = 0
p1 = 0
p2 = 0
p3 = 0
p4 = 0
p5 = 0
p6 = 0
[Reporters]
  [const]
    type = ConstantReporter
    real_names = 'p0 p1 p2 p3 p4 p5 p6'
    real_values = '${p0} ${p1} ${p2} ${p3} ${p4} ${p5} ${p6}'
  []
[]
