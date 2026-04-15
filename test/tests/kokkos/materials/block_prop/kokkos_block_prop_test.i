[Mesh]
  [square]
    type = CartesianMeshGenerator
    dx = '5 5'
    dy = '5 5'
    ix = '5 5'
    iy = '5 5'
    dim = 2
    subdomain_id = '1 2 3 4'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = KokkosBlockPropertyDiffusion
    variable = u
    prop_name = coef
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 3
    value = 0.0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 1.0
  []
[]

[Materials]
  [mat]
    type = KokkosGenericConstantMaterial
    prop_names = 'coef'
    prop_values = '2.0'
    block = '1 3'
  []
  [dummy]
    type = KokkosGenericConstantMaterial
    prop_names = 'notcoef'
    prop_values = '2.0'
    block = '2 4'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
