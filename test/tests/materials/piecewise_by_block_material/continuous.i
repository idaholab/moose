[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
  []
  [A]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 0
    block_name = A
    bottom_left = '0 0 0'
    top_right = '1 1 0'
  []
  [B]
    type = SubdomainBoundingBoxGenerator
    input = A
    block_id = 1
    block_name = B
    bottom_left = '0 0.3 0'
    top_right = '1 0.7 0'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ADMatDiffusion
    variable = u
    diffusivity = D
  []
[]

[BCs]
  [current]
    type = NeumannBC
    variable = u
    boundary = right
    value = 0.002
  []
  [potential]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
[]

[Materials]
  [D]
    type = ADPiecewiseConstantByBlockMaterial
    prop_name = D
    subdomain_to_prop_value = 'A 0.1 B 0.05'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  dt = 0.002
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
