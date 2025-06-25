[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

# Solves a pair of coupled diffusion equations where u=v on the boundary

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 3
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2
  [../]
[]

[KokkosKernels]
  active = 'diff_u diff_v'

  [./diff_u]
    type = KokkosDiffusion
    variable = u
  [../]

  [./diff_v]
    type = KokkosDiffusion
    variable = v
  [../]
[]

[KokkosBCs]
  active = 'right_v left_u'

  [./right_v]
    type = KokkosDirichletBC
    variable = v
    boundary = 1
    value = 3
  [../]

  [./left_u]
    type = KokkosMatchedValueBC
    variable = u
    boundary = 3
    v = v
  [../]
[]

[Preconditioning]
  [./precond]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-10
  l_tol = 1e-12
[]

[Outputs]
  file_base = out_gpu
  exodus = true
[]
