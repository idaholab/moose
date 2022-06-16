[Problem]
  type = FEProblem
  extra_tag_vectors  = 'diff react bodyf'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 15
  xmax = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = u
    diffusivity = k
    extra_vector_tags = 'diff'
  []
  [reaction]
    type = MaterialReaction
    variable = u
    coefficient = alpha
    extra_vector_tags = 'react'
  []
  [source]
    type = BodyForce
    variable = u
    value = 1.0
    extra_vector_tags = 'bodyf'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = k
    prop_values = 1.0
  []
  [alpha]
    type = GenericConstantMaterial
    prop_names = alpha
    prop_values = 1.0
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
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]
