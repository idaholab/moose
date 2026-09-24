coef = 1e12

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

# These are independent copies of the same nonlinear problem. The large equation and its
# boundary condition are multiplied by coef so that row scaling does not change its solution.
[Variables]
  [large]
  []
  [small]
  []
[]

[Kernels]
  [large_diffusion]
    type = KokkosCoefDiffusion
    variable = large
    coef = ${coef}
  []
  [large_reaction]
    type = PReaction
    variable = large
    coefficient = ${coef}
    power = 2
  []
  [large_source]
    type = KokkosBodyForce
    variable = large
    value = ${coef}
  []
  [small_diffusion]
    type = KokkosCoefDiffusion
    variable = small
    coef = 1
  []
  [small_reaction]
    type = PReaction
    variable = small
    coefficient = 1
    power = 2
  []
  [small_source]
    type = KokkosBodyForce
    variable = small
    value = 1
  []
[]

[BCs]
  [large]
    type = RobinBC
    variable = large
    boundary = 'left right'
    coef = ${coef}
    extra_vector_tags = 'ref'
  []
  [small]
    type = RobinBC
    variable = small
    boundary = 'left right'
    coef = 1
    extra_vector_tags = 'ref'
  []
[]

[Convergence]
  [conv]
    type = ReferenceResidualConvergence
    reference_vector = ref
  []
[]

[Executioner]
  type = Steady
  nonlinear_convergence = conv
  automatic_scaling = true
  petsc_options_iname = '-pc_type -ksp_max_it'
  petsc_options_value = 'svd      10'
[]

[Postprocessors]
  [nonlinear_iterations]
    type = NumNonlinearIterations
  []
  [linear_iterations]
    type = NumLinearIterations
  []
[]

[Outputs]
  exodus = true
[]
