# The purpose of this test is to ensure the SimplePredictor resets the std::precision

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
    extra_vector_tags = 'ref'
  [../]
[]

[BCs]
  [./bot]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0.0
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = u
    boundary = top
    function = 't'
  [../]
[]

[Executioner]
  type = Transient

  start_time = 0.0
  dt = 0.5
  end_time = 1.0

  [./Predictor]
    type = SimplePredictor
    scale = 1.0e-10
  [../]
[]
