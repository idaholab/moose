[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  [Partitioner]
    type = PetscExternalPartitioner
    part_package = parmetis
  []
  parallel_type = distributed
  # Need a fine enough mesh to have good partition
  uniform_refine = 1
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
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  [out]
    type = CSV
    execute_on = FINAL
  []
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

[VectorPostprocessors]
  [./nl_wb_element]
    type = WorkBalance
    execute_on = initial
    system = nl
    outputs  = none
    balances = 'num_elems num_nodes'
  []

  [./bl_element]
    type = StatisticsVectorPostprocessor
    vpp = nl_wb_element
    stats = 'min max sum average ratio'
  [../]

  [./nl_wb_edgecuts]
    type = WorkBalance
    execute_on = initial
    system = nl
    outputs  = none
    balances = 'num_partition_sides'
  []

  [./bl_edgecuts]
    type = StatisticsVectorPostprocessor
    vpp = nl_wb_edgecuts
    stats = 'sum'
  [../]
[]
