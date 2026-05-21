[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
[]

[Executioner]
  type = Transient

  [Predictor]
    type = TangentPredictor
    scale = 1
    load_vector_tag = load_increment
  []
[]
