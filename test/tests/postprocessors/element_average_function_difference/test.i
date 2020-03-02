[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 0
    value = -1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./average]
    type = ElementAverageValue
    variable = u
  [../]
  [./diff]
    type = ElementAverageFunctionDifference
    variable = u
    function = x
    point = '0.3 0 0'
  [../]
  [./abs_diff]
    type = ElementAverageFunctionDifference
    variable = u
    function = x
    point = '0.3 0 0'
    absolute_value = true
  [../]
[]

[Outputs]
  csv = true
[]
