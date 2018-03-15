[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  uniform_refine = 2
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
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  # This option appears to modify the behavior in PETSc 3.6.0
  petsc_options = '-pc_hypre_boomeramg_print_statistics'
  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_tol -pc_hypre_boomeramg_max_iter'
  petsc_options_value = 'hypre boomeramg 1e-4 20'
[]

[Outputs]
  exodus = true
[]
