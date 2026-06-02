[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
[]

[Variables]
  [u]
  []
  [lambda_left]
    family = SCALAR
    order = FIRST
  []
  [lambda_bottom]
    family = SCALAR
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[BCs]
  [left_average]
    type = BoundaryIntegralValueConstraint
    variable = u
    boundary = left
    lambda = lambda_left
    phi0 = 0.25
  []
  [bottom_average]
    type = BoundaryIntegralValueConstraint
    variable = u
    boundary = bottom
    lambda = lambda_bottom
    phi0 = 0.5
  []
[]

[Postprocessors]
  [left_average]
    type = SideAverageValue
    variable = u
    boundary = left
    execute_on = timestep_end
  []
  [bottom_average]
    type = SideAverageValue
    variable = u
    boundary = bottom
    execute_on = timestep_end
  []
[]

[Preconditioning]
  [full]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
[]

[Outputs]
  csv = true
  hide = 'lambda_left lambda_bottom'
[]
