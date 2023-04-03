[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
  []
  [pin]
    type = ExtraNodesetGenerator
    input = gen
    new_boundary = 'pin'
    nodes = '0'
  []
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left_neumann]
    type = NeumannBC
    boundary = 'left'
    value = -1
    variable = u
  []
  [right_neumann]
    type = NeumannBC
    boundary = 'right'
    value = 1
    variable = u
  []
  [pin]
    type = ADAverageValuePin
    variable = u
    boundary = 'pin'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Problem]
  # We have to close the matrix before enforcing boundary conditions, which destroys our sparsity pattern
  error_on_jacobian_nonzero_reallocation = false
[]

[Outputs]
  exodus = true
[]
