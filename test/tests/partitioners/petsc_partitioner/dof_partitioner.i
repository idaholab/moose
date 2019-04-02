
[Mesh]
  type = FileMesh
  file = dof_partitioner_in.e
  [Partitioner]
    type = DoFPartitioner
  []
  second_order = true
[]

[Variables]
  # defined everywhere
  [u]
  []

  # defined on block 2
  [v]
    block = 2
  []

  # defined on block 2
  [w]
    order = SECOND
    block = 2
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []

  [diff_v]
    type = Diffusion
    variable = v
  []

  [diff_w]
    type = Diffusion
    variable = w
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

  [right_v]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 1
  []

  [right_w]
    type = DirichletBC
    variable = w
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
  [npid]
    family = Lagrange
    order = first
  []
[]

[AuxKernels]
  [pid_aux]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
  []
  [npid_aux]
    type = ProcessorIDAux
    variable = npid
    execute_on = 'INITIAL'
  []
[]
