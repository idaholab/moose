coef=1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Problem]
  # type = ReferenceResidualProblem
  extra_tag_vectors = 'ref res'
[]

[Variables]
  [u][]
  [v][]
[]

[Kernels]
  [u_diff]
    type = CoefDiffusion
    variable = u
    coef = 1
    extra_vector_tags = 'res'
  []
  [u_rxn]
    type = PReaction
    variable = u
    coefficient = 1
    power = 2
  []
  [u_f]
    type = BodyForce
    variable = u
    value = 2
    extra_vector_tags = 'res'
  []
  [v_diff]
    type = Diffusion
    variable = v
    extra_vector_tags = 'res'
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
    extra_vector_tags = 'res'
  []
[]

[BCs]
  [u]
    type = RobinBC
    boundary = 'left right'
    coef = ${coef}
    variable = u
    extra_vector_tags = 'ref res'
  []
  [v]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = v
    extra_vector_tags = 'ref res'
  []
[]

[Convergence]
  [conv]
    type = ReferenceResidualConvergence
    reference_vector = 'ref'
    residual_vector = 'res'
  []
[]

[Executioner]
  type = Steady
  nonlinear_convergence = conv
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [v_avg]
    type = ElementAverageValue
    variable = v
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
