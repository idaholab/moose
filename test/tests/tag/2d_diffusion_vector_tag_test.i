[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1.0
  [../]
[]

[AuxVariables]

  [./tag_variable1]
    order = FIRST
    family = LAGRANGE
  [../]

  [./tag_variable2]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]

  [./reaction1]
    type = Reaction
    variable = u
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]

  [./reaction2]
    type = Reaction
    variable = u
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]

  [./reaction3]
    type = Reaction
    variable = u
  [../]

  [./reaction4]
    type = Reaction
    variable = u
  [../]

[]

[AuxKernels]

  [./TagVectorAux1]
    type = TagVectorAux
    variable = tag_variable1
    v = u
    vector_tag = vec_tag1
    execute_on = timestep_end
  [../]

  [./TagVectorAux2]
    type = TagVectorAux
    variable = tag_variable2
    v = u
    vector_tag = vec_tag2
    execute_on = timestep_end
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 3
    value = 10
    extra_vector_tags = vec_tag1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 100
    extra_vector_tags = vec_tag2
  [../]

  [./right1]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 100
  [../]

  [./right2]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 100
  [../]
[]

[Problem]
  type = TagTestProblem
  extra_tag_vectors  = 'vec_tag1 vec_tag2'
  test_tag_vectors =  'vec_tag1 vec_tag2'
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = vector_tag_test_out
  exodus = true
[]
