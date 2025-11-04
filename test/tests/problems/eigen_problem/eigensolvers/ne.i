[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    elem_type = QUAD4
    nx = 8
    ny = 8
  []
  [left_block]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '5 10 0'
    block_id = 1
    input = gen
  []
  [refine]
    type = RefineBlockGenerator
    input = left_block
    refinement = '0 0'
    block = '1 0'
  []
[]

# the minimum eigenvalue of this problem is 2*(PI/a)^2;
# Its inverse is 0.5*(a/PI)^2 = 5.0660591821169. Here a is equal to 10.

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./rhs]
    type = CoefReaction
    variable = u
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]
  [./eigen]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  [../]
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNK
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  csv = true
  file_base = monolith_newton
  execute_on = 'timestep_end'
[]
