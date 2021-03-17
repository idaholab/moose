[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1'
    ix = '5 5'
    iy = '5'
    subdomain_id = '1 1'
  []
  [internal_sideset]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x<1.01 & x>0.99'
    included_subdomain_ids = 1
    new_sideset_name = 'center'
    input = 'mesh'
  []
[]

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    block = 1
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
[]

[FVBCs]
  inactive = 'center'
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = FVNeumannBC
    variable = u
    boundary = right
    value = 4
  []
  # Internal center sideset, should cause erroring out
  [center]
    type = FVNeumannBC
    variable = u
    boundary = center
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
