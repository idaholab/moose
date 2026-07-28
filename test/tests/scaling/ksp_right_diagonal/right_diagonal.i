[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 8
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
    variable = u
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
    value = 1
  []
[]

[UserObjects]
  [right_scale]
    type = KSPRightDiagonalScaleTest
    execute_on = PRE_KERNELS
    write_on_rank_zero_only = true
  []
[]

[Postprocessors]
  [average]
    type = ElementAverageValue
    variable = u
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.25
  num_steps = 2
  nl_abs_tol = 1e-12
  nl_max_its = 1
[]

[Outputs]
  csv = true
[]
