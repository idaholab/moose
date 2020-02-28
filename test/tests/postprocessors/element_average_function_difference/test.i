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
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 2
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
    function = 1.5
  [../]
  [./abs_diff]
    type = ElementAverageFunctionDifference
    variable = u
    function = 1.5
    absolute_value = true
  [../]
[]

[Outputs]
  csv = true
[]
