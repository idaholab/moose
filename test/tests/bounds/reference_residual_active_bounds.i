# A negative source drives u below its lower bound of zero over part of the domain, so the
# variational inequality solver holds those degrees of freedom at the bound, where the residual
# stays nonzero. The reference residual convergence check must exclude them to converge.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [bounds_dummy]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    extra_vector_tags = 'ref'
  []
  [source]
    type = BodyForce
    variable = u
    value = -10
    extra_vector_tags = 'ref'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Bounds]
  [u_lower_bound]
    type = ConstantBounds
    variable = bounds_dummy
    bounded_variable = u
    bound_type = lower
    bound_value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -snes_type'
  petsc_options_value = 'lu vinewtonrsls'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-50
  nl_max_its = 20
[]

[Outputs]
  exodus = true
[]
