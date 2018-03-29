# This example demonstrates ability to set Dirichlet boundary conditions for LAGRANGE_VEC variables

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    family = LAGRANGE_VEC
    order = FIRST
  [../]
[]

[Kernels]
  [./diff]
    type = VectorDiffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = LagrangeVecDirichletBC
    variable = u
    values = '0 0 0'
    boundary = 'left'
  [../]
  [./right]
    type = LagrangeVecDirichletBC
    variable = u
    values = '1 1 1'
    boundary = 'right'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options = '-ksp_converged_reason -snes_converged_reason'
[]

[Outputs]
  exodus = true
[]
