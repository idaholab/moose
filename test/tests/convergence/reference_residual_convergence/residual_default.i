coef=3000

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 500
[]

[Problem]
    extra_tag_vectors = 'ref'
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

[Postprocessors]
    [num_nonlinear_it]
      type = NumNonlinearIterations
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
    [./csv]
      type = CSV
    []
[]
