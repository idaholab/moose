[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    subdomain_name = 'BaseMesh'
    dim = 2
    nx = 10
    ny = 10
  []
  [subdomains]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_name = 'Box1'
    bottom_left = '0.1 0.1 0'
    block_id = 1
    top_right = '0.9 0.9 0'
  []
[]

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
  [block]
    type = GenericConstantMaterial
    block = '0 1'
    prop_names = block_prop
    prop_values = 1
  []
  [boundary]
    type = GenericConstantMaterial
    boundary = top
    prop_names = boundary_prop
    prop_values = 2
  []
  [restricted]
    type = GenericConstantMaterial
    block = 1
    prop_names = restricted_prop
    prop_values = 3
  []
[]

[Executioner]
  type = Steady
[]

[Debug]
  show_block_restriction_groups = true
  show_boundary_restriction_groups = true
[]
