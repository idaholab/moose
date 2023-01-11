###########################################################
# This is a simple test of the Postprocessor System. This
# test uses a forcing function and the MMS to verify
# correctness of the implementation.
# Grid adaptivity is applied at successively finer grids
# to verify the correct slope of the measure of error
# against the analytical solution.
#
# @Requirement F6.10
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3

  xmin = 0
  xmax = 2

  ymin = 0
  ymax = 2
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  active = 'forcing_func u_func'

  [./forcing_func]
    type = ParsedFunction
    expression = alpha*alpha*pi*pi*sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '4'
  [../]

  [./u_func]
    type = ParsedFunction
    expression = sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '4'
  [../]
[]

[Kernels]
  active = 'diff forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_func
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = '3'
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  [./Adaptivity]
    refine_fraction = 1.0
    coarsen_fraction = 0.0
    max_h_level = 10
    steps = 4
  [../]
[]

# Postprocessor System
[Postprocessors]
  [./integral]
    type = ElementL2Error
    variable = u
    function = u_func
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = out
  exodus = false
  csv = true
[]
