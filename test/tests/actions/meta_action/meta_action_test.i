###########################################################
# This is a test of the Action System. An Action is created
# to build other objects pro grammatically. Two blocks in
# the input file have been commented out to demonstrate
# usage.
#
# @Requirement F1.50
###########################################################

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
  uniform_refine = 4
[]

# This is our new custom Convection Diffusion "Meta" block
# that adds multiple kernels into our simulation
#
# Convection and Diffusion kernels on the first variable
# Diffusion kernel on the second variable
# The Convection kernel is coupled to the Diffusion kernel on the second variable
[ConvectionDiffusion]
    variables = 'convected diffused'
[]

#[Variables]
#  [./convected]
#  [../]
#  [./diffused]
#  [../]
#[]
#
#[Kernels]
#  [./diff_v]
#    type = Diffusion
#    variable = convected
#  [../]
#  [./diff_u]
#    type = Diffusion
#    variable = diffused
#  [../]
#[]

[BCs]
  active = 'left_convected right_convected left_diffused right_diffused'

  [./left_convected]
    type = DirichletBC
    variable = convected
    boundary = '3'
    value = 0
  [../]

  [./right_convected]
    type = DirichletBC
    variable = convected
    boundary = '1'
    value = 1
  [../]

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '3'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 1
  [../]

[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  file_base = out
[]
