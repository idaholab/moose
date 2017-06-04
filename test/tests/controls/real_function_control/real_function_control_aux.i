###########################################################
# This is a test of the Control Logic System. This test
# uses the RealFunctionControl to change a Kernel
# coefficient based on an analytical function at the end
# of each timestep.
#
# @Requirement F8.10
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./a]
  [../]
[]

[AuxKernels]
  [./a_ak]
    type = ConstantAux
    variable = a
    value = 1
    execute_on = 'initial timestep_begin'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
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

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  dtmin = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Functions]
  [./a_fn]
    type = PiecewiseLinear
    x = '0 1'
    y = '11 1'
  [../]
[]

[Controls]
  [./func_control]
    type = RealFunctionControl
    parameter = 'AuxKernels/a_ak/value'
    function = 'a_fn'
    execute_on = 'initial timestep_begin'
  [../]
[]
