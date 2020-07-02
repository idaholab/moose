[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    xmax = 2
    ny = 2
    ymax = 2
    nz = 2
    zmax = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 1 1'
    block_id = 1
  [../]
  [./break_boundary]
    input = subdomain1
    type = BreakBoundaryOnSubdomainGenerator
  [../]
  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = break_boundary
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]

[Kernels]
  [./diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 0
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
  [./source_u]
    type = BodyForce
    variable = u
    value = 1
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]
[]

[BCs]
  [./u]
    type = VacuumBC
    variable = u
    boundary = 'left_to_0 bottom_to_0 back_to_0 right top front'
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
  [./v]
    type = VacuumBC
    variable = v
    boundary = 'left_to_1 bottom_to_1 back_to_1'
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
[]

[AuxVariables]
  [./tag_variable1]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]

  [./tag_variable2]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]

[AuxKernels]
  [./TagVectorAux1]
    type = TagVectorAux
    variable = tag_variable1
    v = u
    block = 0
    vector_tag = vec_tag2
    execute_on = timestep_end
  [../]

  [./TagVectorAux2]
    type = TagMatrixAux
    variable = tag_variable2
    v = v
    block = 1
    matrix_tag = mat_tag2
    execute_on = timestep_end
  [../]
[]

[Postprocessors]
  [./u_int]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 0
  [../]
  [./v_int]
    type = ElementIntegralVariablePostprocessor
    variable = v
    block = 1
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Problem]
  type = TagTestProblem
  test_tag_vectors =  'nontime residual vec_tag1 vec_tag2'
  test_tag_matrices = 'mat_tag1 mat_tag2'

  extra_tag_matrices = 'mat_tag1 mat_tag2'
  extra_tag_vectors  = 'vec_tag1 vec_tag2'
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
