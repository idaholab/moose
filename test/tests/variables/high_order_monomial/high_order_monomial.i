###########################################################
# This is a simple test demonstrating the use of the
# Higher order monomial variable type.
#
# @Requirement F3.10
###########################################################

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./u]
  [../]
[]

# Monomial variable types
[AuxVariables]
  [./first]
    family = MONOMIAL
  [../]
  [./second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./third]
    order = THIRD
    family = MONOMIAL
  [../]
[]

[Functions]
  [./first]
    type = ParsedFunction
    expression = 1+2*x+2*y
  [../]
  [./second]
    type = ParsedFunction
    expression = 1+2*x+4*x*x+2*y+4*y*y+4*x*y
  [../]
  [./third]
    type = ParsedFunction
    expression = 1+2*x+4*x*x+8*x*x*x+2*y+4*y*y+8*y*y*y+4*x*y+8*x*x*y
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./first]
    type = FunctionAux
    variable = first
    function = first
    execute_on = timestep_end
  [../]
  [./second]
    type = FunctionAux
    variable = second
    function = second
    execute_on = timestep_end
  [../]
  [./third]
    type = FunctionAux
    variable = third
    function = third
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [./first_error]
    type = ElementL2Error
    variable = first
    function = first
    execute_on = 'initial timestep_end'
  [../]
  [./second_error]
    type = ElementL2Error
    variable = second
    function = second
    execute_on = 'initial timestep_end'
  [../]
  [./third_error]
    type = ElementL2Error
    variable = third
    function = third
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
