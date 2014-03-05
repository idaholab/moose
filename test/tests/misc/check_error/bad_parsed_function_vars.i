[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 2
[]

[Variables]
  [./u]
    block = 0
  [../]
[]

[Functions]
  [./sin_func]
    type = ParsedFunction
    value = sin(y)
    vars = y        # <- This is a bad - you can't specify x, y, z, or t
  [../]
[]

[Kernels]
  [./diffused]
    type = Diffusion
    variable = u
    block = 0
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
  [./right]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = sin_func
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
