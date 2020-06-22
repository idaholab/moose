[Problem]
  type = FEProblem
  extra_tag_vectors  = 'diff react bodyf'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 30
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
    diffusivity = D
    extra_vector_tags = 'diff'
  []
  [absorption]
    type = MaterialReaction
    variable = u
    coefficient = sig
    extra_vector_tags = 'react'
  []
  [source]
    type = BodyForce
    variable = u
    value = 1.0
    extra_vector_tag = 'bodyf'
  []
[]

[Materials]
  [diffusivity]
    type = GenericConstantMaterial
    prop_names = D
    prop_values = 1.0
  []
  [xs]
    type = GenericConstantMaterial
    prop_names = sig
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

[Outputs]
[]
