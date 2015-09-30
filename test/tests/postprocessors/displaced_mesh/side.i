[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  displacements = 'ux uy'
[]

[AuxVariables]
  [./ux]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
  [./uy]
    [./InitialCondition]
      type = FunctionIC
      function = y
    [../]
  [../]
  [./c]
    initial_condition = 1
  [../]
[]

[Variables]
  [./a]
  [../]
[]
[Kernels]
  [./a]
    type = Diffusion
    variable = a
  [../]
[]

[Postprocessors]
  [./without]
    type = SideIntegralVariablePostprocessor
    variable = c
    execute_on = initial
    boundary = left
  [../]
  [./with]
    type = SideIntegralVariablePostprocessor
    variable = c
    use_displaced_mesh = true
    execute_on = initial
    boundary = left
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]
