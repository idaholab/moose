coef=1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Problem]
  # type = ReferenceResidualProblem
  extra_tag_vectors = 'u_ref u_res v_ref v_res'
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
    extra_vector_tags = 'u_res'
  []
  [u_rxn]
    type = PReaction
    variable = u
    coefficient = 1
    power = 2
    extra_vector_tags = 'u_res'
  []
  [u_f]
    type = BodyForce
    variable = u
    value = 1
    extra_vector_tags = 'u_res'
  []
  [v_diff]
    type = Diffusion
    variable = v
    extra_vector_tags = 'v_res'
  []
  [v_rxn]
    type = PReaction
    variable = v
    coefficient = 1
    power = 2
    extra_vector_tags = 'v_res'
  []
  [v_f]
    type = BodyForce
    variable = v
    value = 1
    extra_vector_tags = 'v_res'
  []
[]

[BCs]
  [u]
    type = RobinBC
    boundary = 'left right'
    coef = ${coef}
    variable = u
    extra_vector_tags = 'u_ref u_res'
  []
  [v]
    type = RobinBC
    boundary = 'left right'
    coef = 1
    variable = v
    extra_vector_tags = 'v_ref v_res'
  []
[]

[Convergence]
  [u_conv]
    type = ReferenceResidualConvergence
    reference_vector = 'u_ref'
    residual_vector = 'u_res'
  []
  [v_conv]
    type = ReferenceResidualConvergence
    reference_vector = 'v_ref'
    residual_vector = 'v_res'
  []
  [conv]
    type = ParsedConvergence
    convergence_expression = 'u_conv & v_conv'
    symbol_names = 'u_conv v_conv'
    symbol_values = 'u_conv v_conv'
  []
[]

[Executioner]
  type = Steady
  nonlinear_convergence = conv
[]

[Outputs]
  exodus = true
[]
