###########################################################
# This is a simple test demonstrating the use of the
# Hierarchic variable type.
#
# @Requirement F3.10
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 5
  elem_type = EDGE3
[]

[Functions]
  [./bc_fnl]
    type = ParsedFunction
    expression = -1
  [../]
  [./bc_fnr]
    type = ParsedFunction
    expression = 1
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = x
  [../]

  [./solution]
    type = ParsedGradFunction
    expression = x
    grad_x = 1
  [../]
[]

# Hierarchic Variable type
[Variables]
  [./u]
    order = FIRST
    family = HIERARCHIC
  [../]
[]

[Kernels]
  active = 'diff forcing reaction'
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./reaction]
    type = Reaction
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./bc_left]
    type = FunctionNeumannBC
    variable = u
    boundary = 'left'
    function = bc_fnl
  [../]
  [./bc_right]
    type = FunctionNeumannBC
    variable = u
    boundary = 'right'
    function = bc_fnr
  [../]
[]

[Postprocessors]
  [./dofs]
    type = NumDOFs
  [../]

  [./h]
    type = AverageElementSize
  [../]

  [./L2error]
    type = ElementL2Error
    variable = u
    function = solution
  [../]
  [./H1error]
    type = ElementH1Error
    variable = u
    function = solution
  [../]
  [./H1Semierror]
    type = ElementH1SemiError
    variable = u
    function = solution
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-11

  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
