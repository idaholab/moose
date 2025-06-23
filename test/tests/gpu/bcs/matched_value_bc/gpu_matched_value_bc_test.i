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

[GPUKernels]
  active = 'diff_u diff_v'

  [./diff_u]
    type = GPUDiffusion
    variable = u
  [../]

  [./diff_v]
    type = GPUDiffusion
    variable = v
  [../]
[]

[GPUBCs]
  active = 'right_v left_u'

  [./right_v]
    type = GPUDirichletBC
    variable = v
    boundary = 1
    value = 3
  [../]

  [./left_u]
    type = GPUMatchedValueBC
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
