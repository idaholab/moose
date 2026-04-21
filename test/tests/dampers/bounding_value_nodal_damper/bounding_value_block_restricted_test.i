[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 1
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
  []
  [left_block]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0.0 0.0 0.0'
    top_right = '0.1 1.0 0.0'
  []
  [right_block]
    type = SubdomainBoundingBoxGenerator
    input = left_block
    block_id = 2
    bottom_left = '0.1 0.0 0.0'
    top_right = '1.0 1.0 0.0'
  []
[]

[Variables]
  [theta_m]
    family = LAGRANGE
    order = FIRST
    block = 1
    initial_condition = 0.0
  []
  [dummy]
    family = LAGRANGE
    order = FIRST
    block = 2
    initial_condition = 0.0
  []
[]

[Kernels]
  [theta_td]
    type = TimeDerivative
    variable = theta_m
    block = 1
  []
  [theta_src]
    type = BodyForce
    variable = theta_m
    value = 1.0
    block = 1
  []
  [dummy_td]
    type = TimeDerivative
    variable = dummy
    block = 2
  []
[]

[Dampers]
  [limit_theta]
    type = BoundingValueNodalDamper
    variable = theta_m
    min_value = -0.1
    max_value = 0.25
    min_damping = 1e-8
  []
[]

[BCs]
  [dummy_fix]
    type = DirichletBC
    variable = dummy
    boundary = 'right'
    value = 0.0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1.0
  num_steps = 1
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  l_tol = 1e-14
[]

[Outputs]
  exodus = true
[]