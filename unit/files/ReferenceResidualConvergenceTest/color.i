[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[GlobalParams]
  absolute_value_vector_tags = ref
[]

[Problem]
  extra_tag_vectors = ref
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = BodyForce
    variable = u
    value = 1
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

[Convergence]
  [reference]
    type = ReferenceResidualConvergence
    reference_vector = ref
    # The direct linear solve drives both quantities below these standard convergence tolerances.
    nl_abs_tol = 1e-9
    nl_rel_tol = 1e-9
  []
[]

[Executioner]
  type = Steady
  nonlinear_convergence = reference
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = false
[]
