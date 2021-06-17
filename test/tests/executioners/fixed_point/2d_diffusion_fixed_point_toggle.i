[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [pseudo_time]
    type = CoupledForceLagged
    variable = u
    coefficient = 0.1
    v = u
    tag = 'previous'
  []
  [pseudo_time_compensation]
    type = CoefReaction
    variable = u
    coefficient = 0.1
  []
[]

[BCs]
  [left]
    type = VacuumBC
    variable = u
    boundary = left
  []

  [right]
    type = NeumannBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [unorm]
    type = ElementL2Norm
    variable = u
  []
  [udiff]
    type = ElementL2Diff
    variable = u
    tag = 'previous'
  []
[]

[Problem]
  type = FixedPointProblem
  fp_tag_name = 'previous'
  tagged_vector_for_partial_residual = false
[]

[Executioner]
  type = FixedPointSteady
  nl_rel_tol = 1e-2
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
