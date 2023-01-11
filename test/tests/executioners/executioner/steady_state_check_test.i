#
# Run transient simulation into steady state
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD9
[]

[Variables]
  active = 'u'

  [./u]
    order = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    # dudt = 3*t^2*(x^2 + y^2)
#    expression = 3*t*t*((x*x)+(y*y))-(4*t*t*t)
    expression = -4
  [../]

  [./exact_fn]
    type = ParsedFunction
#    expression = t*t*t*((x*x)+(y*y))
    expression = ((x*x)+(y*y))
  [../]
[]

[Kernels]
  active = 'diff ie ffn'

  [./ie]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'

  solve_type = 'PJFNK'

  nl_abs_tol = 1e-14

  start_time = 0.0
  num_steps = 12
  dt = 1

  steady_state_detection = true
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_ss_check
  exodus = true
[]
