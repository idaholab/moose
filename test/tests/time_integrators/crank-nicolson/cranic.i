#
# Testing a solution that is second order in space and second order in time
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
    expression = 2*t*((x*x)+(y*y))-(4*t*t)
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = t*t*((x*x)+(y*y))
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
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'crank-nicolson'

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
