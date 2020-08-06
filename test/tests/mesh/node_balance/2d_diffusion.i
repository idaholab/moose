[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  [Partitioner]
    type = GridPartitioner
  []
  mesh_node_balance = true
  parallel_type = distributed
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./pid]
    order = FIRST
    family = LAGRANGE
  []
  [./epid]
    order = CONSTANT
    family = monomial
  []
[]

[AuxKernels]
  [./pidaux]
    type = ProcessorIDAux
    variable = pid
  [../]

  [./epidaux]
    type = ProcessorIDAux
    variable = epid
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
