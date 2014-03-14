[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./left]
    type = ParsedFunction
    value = 'y'
  [../]

  [./right]
    type = ParsedFunction
    value = '1+y'
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

   [./left]
    type = FunctionPresetBC
    variable = u
    boundary = 3
    function = left
  [../]

  [./right]
    type = FunctionPresetBC
    variable = u
    boundary = 1
    function = right
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  linear_residuals = true
  file_base = bc_func_out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
