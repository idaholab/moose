[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmin = -1
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  [./u]
  [../]

  [./v]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[Functions]
  [./leg1]
    type = ParsedFunction
    expression = 'x'
  [../]

  [./leg2]
    type = ParsedFunction
    expression = '0.5*(3.0*x*x-1.0)'
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 2
    value = 1
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    preset = false
    boundary = 1
    value = 200
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    preset = false
    boundary = 2
    value = 100
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

  # this is large on purpose so we don't reduce the variable residual to machine zero
  # and so that we can compare to larger numbers. This also means this test can run only
  # in serial, since parallel runs yield different convergence history.
  nl_rel_tol = 1e-4
[]

[Postprocessors]
  [./u_res_l2]
    type = VariableResidual
    variable = u
  [../]

  [./v_res_l2]
    type = VariableResidual
    variable = v
  [../]
[]

[Outputs]
  csv = true
  [./console]
    type = Console
    # turn this on, so we can visually compare the postprocessor value with what is computed by the Console object
    all_variable_norms = true
  [../]
[]
