[Mesh]
  [generate_2d]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [extrude]
    type = MeshExtruderGenerator
    input = generate_2d
    extrusion_vector = '0 0 1'
    num_layers = 5
  []
  [Partitioner]
    type = GridPartitioner
    nx = 1
    ny = 1
    nz = 4
    # So we can use the same partitioner for the 2D and 3D mesh
    grid_computation = automatic
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
