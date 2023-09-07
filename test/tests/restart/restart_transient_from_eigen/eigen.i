[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 10
[]

[Variables/u]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [rhs]
    type = Reaction
    extra_vector_tags = 'eigen'
    variable = u
    rate = -1
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [eigen]
    type = EigenDirichletBC
    variable = u
    boundary = left
  []
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
  []
[]

[Executioner]
  type = Eigenvalue
[]

[Outputs]
  exodus = true
  checkpoint = true
[]
