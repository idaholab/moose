[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    extra_vector_tags = ref
  [../]
  [./dt]
    type = TimeDerivative
    variable = u
    extra_vector_tags = ref
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = left
    value = -1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  line_search = none
  nl_forced_its = 10
  num_steps = 1
[]
