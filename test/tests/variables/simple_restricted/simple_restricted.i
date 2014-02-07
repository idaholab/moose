[Mesh]
  type = FileMesh
  file = simple_multiblock.e
  dim = 2
[]

[Variables]
  [./u]
  [../]
  [./v]
    block = right
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
    block = right
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./middle_v]
    type = DirichletBC
    variable = v
    boundary = middle
    value = 3
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 4
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  linear_residuals = true
  output_initial = true
  exodus = true
  perf_log = true
[]

