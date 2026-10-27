[Mesh]
  type = FileMesh
  file = block_weighted_partitioner.e

  [Partitioner]
    type = BlockWeightedPartitioner
    block = '1 2 3'
    weight = '3 1 10'
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
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [sum_sides]
    type = StatVector
    stat = sum
    object = nl_wb_element
    vector = num_partition_sides
  []
  [min_elems]
    type = StatVector
    stat = min
    object = nl_wb_element
    vector = num_elems
  []
  [max_elems]
    type = StatVector
    stat = max
    object = nl_wb_element
    vector = num_elems
  []
[]

[VectorPostprocessors]
  [nl_wb_element]
    type = WorkBalance
    execute_on = initial
    system = nl
    balances = 'num_elems num_partition_sides'
    outputs = none
  []
[]

[Outputs]
  csv = true
[]
