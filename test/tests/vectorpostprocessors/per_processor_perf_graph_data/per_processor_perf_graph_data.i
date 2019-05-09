[Mesh]
  type = FileMesh
  file = biased_mesh.e
  [Partitioner]
    type = GridPartitioner
    bias_x = 1
    nx = 3
    ny = 1
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [kernel_time]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [kernel_time]
    type = VectorPostprocessorVisualizationAux
    variable = kernel_time
    execute_on = 'timestep_end'
    vpp = 'pppg'
    vector_name = NonlinearSystemBase::Kernels::SELF
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

[VectorPostprocessors]
  [pppg]
    type = PerProcessorPerfGraphData
    data_types = 'self'
    section_names = 'NonlinearSystemBase::Kernels'
  []
[]

[Outputs]
  exodus = true
  csv = true
  [pg]
    type = PerfGraphOutput
    execute_on = 'final'
    level = 8
  []
[]
