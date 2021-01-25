[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  parallel_type = REPLICATED
  partitioner = linear
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
[]

[AuxVariables]
  [num_elems]
    family = MONOMIAL
    order = CONSTANT
  []
  [partition_surface_area]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [wb_num_elems]
    type = VectorPostprocessorVisualizationAux
    vpp = 'wb'
    vector_name = num_elems
    variable = num_elems
    execute_on = 'TIMESTEP_END'
  []
  [wb_partition_surface_area]
    type = VectorPostprocessorVisualizationAux
    vpp = 'wb'
    vector_name = partition_surface_area
    variable = partition_surface_area
    execute_on = 'TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [wb]
    type = WorkBalance
    sync_to_all_procs = 'true'
    execute_on = 'INITIAL'
  []
[]
