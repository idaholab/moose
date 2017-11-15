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

# the minimum eigenvalue is (2*PI*(p-1)^(1/p)/a/p/sin(PI/p))^p;
# Its inverse is 35.349726539758187. Here a is equal to 10.

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]


[Kernels]
  [./diff]
    type = PHarmonic
    variable = u
    p = 3
    vector_tags = 'nontime Ax_tag'
  [../]

  [./rhs]
    type = PMassKernel
    eigen_kernel = true
    variable = u
    p = 3
    vector_tags = 'nontime Bx_tag'
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 2'
    value = 0
    vector_tags = 'nontime Ax_tag'
  [../]
  [./eigen]
    type = DirichletBC
    variable = u
    boundary = '0 2'
    value = 0
    zero_residual = true
    vector_tags = 'Bx_tag'
  [../]
[]

[Executioner]
  type = Eigenvalue
  solve_type = MF_MONOLITH_NEWTON
  eigen_problem_type = GEN_NON_HERMITIAN
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
