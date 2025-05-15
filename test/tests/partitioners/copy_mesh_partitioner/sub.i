[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10

  [Partitioner]
    type = CopyMeshPartitioner
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
  [td]
    type = TimeDerivative
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

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.01

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  hide = 'parent_pid pid'
[]

[AuxVariables]
  [pid]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = ProcessorIDAux
    []
  []
  [parent_pid]
    order = CONSTANT
    family = MONOMIAL
  []
  [diff_pids]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = ParsedAux
      expression = 'pid - parent_pid'
      coupled_variables = 'pid parent_pid'
    []
  []
[]
