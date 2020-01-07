coef=1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
  [u][]
  [v][]
[]

[Kernels]
  [u_diff]
    type = CoefDiffusion
    variable = u
    coef = ${coef}
  []
  [u_rxn]
    type = PReaction
    variable = u
    coefficient = ${coef}
    power = 2
  []
  [u_f]
    type = BodyForce
    variable = u
    value = ${coef}
  []
  [v_diff]
    type = Diffusion
    variable = v
  []
  [v_rxn]
    type = PReaction
    variable = v
    coefficient = 1
    power = 2
  []
  [v_f]
    type = BodyForce
    variable = v
    value = 1
  []
[]

[BCs]
  [u]
    type = RobinBC
    boundary = 'left right'
    coef = ${coef}
    variable = u
    extra_vector_tags = 'ref'
  []
  [v]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = v
    extra_vector_tags = 'ref'
  []
[]


[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
