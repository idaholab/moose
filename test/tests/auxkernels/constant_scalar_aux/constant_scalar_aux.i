#
# Testing a solution that is second order in space and first order in time
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

[AuxVariables]
  [./x]
    family = SCALAR
    order = FIRST
  [../]
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]
[]

[ICs]
  [./ic_x]
    type = ScalarConstantIC
    variable = x
    value = 11
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = ((x*x)+(y*y))-(4*t)
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*((x*x)+(y*y))
  [../]
[]

[AuxScalarKernels]
  [./const_x]
    type = ConstantScalarAux
    variable = x
    value = 11
  [../]
[]

[Kernels]
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
    execute_on = 'initial timestep_end'
  [../]

[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'

  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 5
  dt = 0.25

#  [./Adaptivity]
#    refine_fraction = 0.2
#    coarsen_fraction = 0.3
#    max_h_level = 4
#  [../]
[]

[Outputs]
  exodus = true
[]
