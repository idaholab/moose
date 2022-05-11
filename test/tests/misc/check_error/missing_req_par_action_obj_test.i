[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]

  uniform_refine = 4
[]

# This meta-Action is not a MooseObjectAction so we are testing
# missing required parameters on standard Actions (variables)
[ConvectionDiffusion]
[]

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

    some_var = diffused
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
  file_base = out
[]
