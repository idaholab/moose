[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    nx = 8
    ny = 8
    elem_type = QUAD4
  []
[]

# the minimum eigenvalue is (2*PI*(p-1)^(1/p)/a/p/sin(PI/p))^p;
# Its inverse is 35.349726539758187. Here a is equal to 10.

[Variables]
  [./u]
  []
[]

# Set an random initial condition is necessary for this problem
# A constant initial condition will not work for this problem since
# the problem is ill-conditioned at a constant vector.
# We observed bad convergence when using a constant initial condition
[ICs]
  [./uic]
    type = RandomIC
    variable = u
  [../]
[]

[Kernels]
  [./diff]
    type = PHarmonic
    variable = u
    p = 3
  [../]

  [./rhs]
    type = PMassKernel
    extra_vector_tags = 'eigen'
    variable = u
    coefficient = -1.0
    p = 3
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 2'
    value = 0
  [../]
  [./eigen]
    type = EigenDirichletBC
    variable = u
    boundary = '0 2'
  [../]
[]

[Executioner]
  type = Eigenvalue
  free_power_iterations = 10
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
  file_base = ane
  execute_on = 'timestep_end'
[]
