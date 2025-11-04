[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    elem_type = QUAD4
    nx = 8
    ny = 8
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = D
  []
  [rhs]
    type = MatReaction
    variable = u
    reaction_rate = L
    extra_vector_tags = 'eigen'
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = 'D L'
    prop_values = 'diff_fun react_fun'
  []
[]

[Functions]
  [diff_fun]
    type = ConstantFunction
    value = 1
  []
  [react_fun]
    type = ConstantFunction
    value = 1
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  []
  [eigen]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  []
[]

[Executioner]
  type = Eigenvalue
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
  []
[]

[Postprocessors]
  [eigenvalue]
    type = VectorPostprocessorComponent
    vectorpostprocessor = eigenvalues
    vector_name = eigen_values_real
    index = 0
  []
[]
