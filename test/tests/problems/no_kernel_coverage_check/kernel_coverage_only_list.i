[Problem]
  solve = true # coverage check needs solve to be true

  kernel_coverage_check = 'only_list'
  kernel_coverage_block_list = 'BaseMesh'

  material_coverage_check = false
[]

[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    subdomain_name = 'BaseMesh'
    dim = 2
    nx = 10
    ny = 10
  []
  add_subdomain_names = 'empty_subdomain'
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = 'BaseMesh'
  []
  [diff2]
    type = MatDiffusion
    diffusivity = 1e-4
    variable = u
    block = 'BaseMesh'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
