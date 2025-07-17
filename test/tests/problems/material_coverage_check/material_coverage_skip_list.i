[Problem]
  solve = true # coverage check needs solve to be true

  kernel_coverage_check = false

  material_coverage_check = 'skip_list'
  material_coverage_block_list = 'empty_subdomain'
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
    block = 'BaseMesh'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
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

[Materials]
  [diff]
    type = GenericConstantMaterial
    prop_names = 'rxn_coeff'
    prop_values = '-2'
    block = 'BaseMesh'
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
