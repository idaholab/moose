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
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = bc_func_out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]
