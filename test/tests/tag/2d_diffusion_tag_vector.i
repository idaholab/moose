[Mesh]
  file = square.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
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
  [./diff]
    type = Diffusion
    variable = u
    extra_vector_tags = 'vec_tag1 vec_tag2'
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
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
    extra_vector_tags = vec_tag1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
    extra_vector_tags = vec_tag2
  [../]
[]

[Problem]
  type = FEProblem
  extra_tag_vectors  = 'vec_tag1 vec_tag2'
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = tag_vector_out
  exodus = true
[]
