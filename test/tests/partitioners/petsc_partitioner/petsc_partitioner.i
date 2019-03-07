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
