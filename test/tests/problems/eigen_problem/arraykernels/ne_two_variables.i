[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  elem_type = QUAD4
  nx = 8
  ny = 8
[]

# the minimum eigenvalue of this problem is 2*(PI/a)^2;
# Its inverse is 0.5*(a/PI)^2 = 5.0660591821169. Here a is equal to 10.

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diffu]
    type = Diffusion
    variable = u
  [../]

  [./diffv]
    type = Diffusion
    variable = v
  [../]

  [./rhsu]
    type = CoefReaction
    variable = u
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  [../]

  [./rhsv]
    type = CoefReaction
    variable = v
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  [../]
[]

[BCs]
  [./homogeneousu]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]
  [./homogeneousv]
    type = DirichletBC
    variable = v
    boundary = '0 1 2 3'
    value = 0
  [../]

  [./eigenu]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  [../]
  [./eigenv]
    type = EigenDirichletBC
    variable = v
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
  execute_on = 'timestep_end'
[]
