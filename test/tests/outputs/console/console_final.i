###########################################################
# This test exercises console Output control. The console
# output is only output every third step. Additionally it
# is forced to be output after the final timestep as well.
#
# @Requirement U1.40
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
[]

[Functions]
  [./ffn]
    type = ParsedFunction
    expression = -4
  [../]

  [./exactfn]
    type = ParsedFunction
    expression = x*x+y*y
  [../]

  [./aux_exact_fn]
    type = ParsedFunction
    expression = t*(x*x+y*y)
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./force]
    type = BodyForce
    variable = u
    function = ffn
  [../]
[]

[AuxVariables]
  [./aux_u]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./a]
    type = FunctionAux
    variable = aux_u
    function = aux_exact_fn
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exactfn
  [../]
[]

[Postprocessors]
  [./elem_56]
    type = ElementalVariableValue
    variable = u
    elementid = 56
  [../]

  [./aux_elem_99]
    type = ElementalVariableValue
    variable = aux_u
    elementid = 99
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  dt = 0.01
  start_time = 0
  num_steps = 10
[]

[Outputs]
  interval = 3
  execute_on = 'initial timestep_end final'
[]
