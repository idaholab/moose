# No Mesh Block!

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [leftBC]
    type = DirichletBC
    variable = u
    boundary = 10
    value = 1
  []
  [rightBC]
    type = DirichletBC
    variable = u
    boundary = 11
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
