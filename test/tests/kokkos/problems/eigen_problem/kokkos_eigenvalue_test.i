[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  elem_type = QUAD4
  nx = 8
  ny = 8
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [rhs]
    type = KokkosReaction
    variable = u
    rate = -1.0
    extra_vector_tags = 'eigen'
  []
[]

[BCs]
  [homogeneous]
    type = KokkosDirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  []
  [eigen]
    type = KokkosEigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  []
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNK
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
