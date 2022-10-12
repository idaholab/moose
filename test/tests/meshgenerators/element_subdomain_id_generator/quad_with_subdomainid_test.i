[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    nz = 0
    zmin = 0
    zmax = 0
    elem_type = QUAD4
  []

  [subdomain_id]
    type = ElementSubdomainIDGenerator
    input = gen
    subdomain_ids = '0 1
                     1 1'
  []
[]

[Variables]
  active = 'u'

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  active = 'diff'

  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  active = 'left right'

  # Mesh Generation produces boundaries in counter-clockwise fashion
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out_quad_subdomain_id
  exodus = true
[]
