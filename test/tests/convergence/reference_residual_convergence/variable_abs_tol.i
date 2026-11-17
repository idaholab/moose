# Two decoupled variables whose residuals differ in magnitude by the factor coef. The round-off floor
# of each residual scales with its magnitude, so no single absolute tolerance is both above the floor
# of u and tight enough for v. The relative tolerance is zero, so only absolute tolerances decide
# convergence.

coef = 1e8

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Variables]
  [u]
  []
  [v]
  []
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
    coefficient = ${coef}
    variable = u
    extra_vector_tags = 'ref'
  []
  [v]
    type = RobinBC
    boundary = 'left right'
    coefficient = 1
    variable = v
    extra_vector_tags = 'ref'
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
  [v_avg]
    type = ElementAverageValue
    variable = v
  []
[]

[Convergence]
  [conv]
    type = ReferenceResidualConvergence
    reference_vector = 'ref'
    nl_rel_tol = 0
    nl_abs_tol = 1e-12
    variable_nl_abs_tol = 'u 1e-4'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_max_its = 10
  nonlinear_convergence = conv
[]

[Outputs]
  csv = true
[]
