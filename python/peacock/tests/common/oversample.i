###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of a "Transient" Executioner.
#
# @Requirement F1.10
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
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
    expression = 3*t*t*((x*x)+(y*y))-(4*t*t*t)
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*t*t*((x*x)+(y*y))
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
  active = 'all'

  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]

  [./dt]
    type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 5
  dt = 0.1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_transient
  exodus = true
  [./refine_2]
    type = Exodus
    file_base = oversample_2
    refinements = 2
  [../]
[]
