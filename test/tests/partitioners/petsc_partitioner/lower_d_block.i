# The mesh file holds a 10x10 QUAD4 block and an EDGE2 lower-dimensional block on its left side.
# The lower-dimensional block must be in the file: one added by a mesh generator does not exist yet
# when the mesh is first partitioned.
[Mesh]
  [file]
    type = FileMeshGenerator
    file = lower_d_block.e
  []
  [Partitioner]
    type = PetscExternalPartitioner
    # Keep ParMETIS; the automatic switch to PTScotch for small meshes depends on the rank count
    part_package = parmetis
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
  solve_type = NEWTON
[]
