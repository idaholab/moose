[BCs]
  [./left]
    boundary = 'left'
    type = DirichletBC
    variable = u
    value = 0
  [../]
  [./right]
    boundary = 'right'
    type = DirichletBC
    variable = u
    value = 1
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  solve_type = PJFNK
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Outputs]
  exodus = true
[]

[Variables]
  [./u]
  [../]
[]

