[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
  []
[]

[Variables]
  [u]
    family = LAGRANGE_VEC
  []
[]

[Kernels]
  [diff]
    type = VectorDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = VectorPenaltyDirichletBC
    variable = u
    boundary = 'left'
    x_exact_sln = '-1'
    y_exact_sln = '-2'
    z_exact_sln = '-3'
    penalty = 1e6
  []
  [right]
    type = VectorPenaltyDirichletBC
    variable = u
    boundary = 'right'
    x_exact_sln = '1'
    y_exact_sln = '2'
    z_exact_sln = '3'
    penalty = 1e6
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -ksp_norm_type'
  petsc_options_value = 'hypre    preconditioned'
[]

[Outputs]
  exodus = true
[]
