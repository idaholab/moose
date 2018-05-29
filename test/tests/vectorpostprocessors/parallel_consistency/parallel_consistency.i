[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10

  # To make this deterministic
  [Partitioner]
    type = GridPartitioner
    nx = 2
    ny = 1
    nz = 1
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

[VectorPostprocessors]
  [constant]
    type = ConstantVectorPostprocessor
    value = '3 4'
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

[AuxVariables]
  [scattered]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [viewit]
    type = VectorPostprocessorVisualizationAux
    vpp = 'constant'
    vector_name = value
    variable = scattered
    execute_on = 'TIMESTEP_END'
  []
[]
