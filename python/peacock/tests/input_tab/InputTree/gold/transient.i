# ##########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of a "Transient" Executioner.
#
# @Requirement F1.10
# ##########################################################
[BCs]
  active = 'all'
  [./all]
    boundary = '0 1 2 3'
    type = FunctionDirichletBC
    variable = u
    function = exact_fn
  [../]
  [./left]
    boundary = '3'
    type = DirichletBC
    variable = u
    value = 0
  [../]
  [./right]
    boundary = '1'
    type = DirichletBC
    variable = u
    value = 1
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  dt = 0.1
  num_steps = 5
  scheme = implicit-euler
  solve_type = PJFNK
  start_time = 0.0
[]

[Functions]
  [./forcing_fn]
    # dudt = 3*t^2*(x^2 + y^2)
    type = ParsedFunction
    value = '3*t*t*((x*x)+(y*y))-(4*t*t*t)'
  [../]
  [./exact_fn]
    type = ParsedFunction
    value = 't*t*t*((x*x)+(y*y))'
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
    type = UserForcingFunction
    function = forcing_fn
    variable = u
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD4
  nx = 10
  ny = 10
  xmax = 1
  xmin = -1
  ymax = 1
  ymin = -1
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  file_base = out_transient
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    function = exact_fn
    variable = 'u'
  [../]
  [./dt]
    type = TimestepSize
  [../]
[]

[Variables]
  active = 'u'
  [./u]
    family = LAGRANGE
    order = FIRST
    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]
[]

