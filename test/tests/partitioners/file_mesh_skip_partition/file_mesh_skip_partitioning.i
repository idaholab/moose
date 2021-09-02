[Mesh]
  [generate_2d]
    type = FileMeshGenerator
    file = 2d_base.e
    skip_partitioning = true
  []
  [extrude]
    type = MeshExtruderGenerator
    input = generate_2d
    extrusion_vector = '0 0 1'
    num_layers = 5
  []
  [Partitioner]
    type = HierarchicalGridPartitioner
    nx_nodes = 2
    ny_nodes = 1
    nz_nodes = 1

    nx_procs = 1
    ny_procs = 1
    nz_procs = 2
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [pid]
    type = ProcessorIDAux
    variable = pid
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
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
