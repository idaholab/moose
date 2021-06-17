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

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
    eigen = true
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]

  [./rhs]
    type = CoupledForce
    variable = u
    v = v
    extra_vector_tags = 'eigen'
  [../]
  [./src_v]
    type = CoupledForce
    variable = v
    v = u
  [../]
[]

[BCs]
  [./homogeneous_u]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]
  [./homogeneous_v]
    type = DirichletBC
    variable = v
    boundary = '0 1 2 3'
    value = 0
  [../]
  [./eigenBC_u]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  [../]
  [./eigenBC_v]
    type = EigenDirichletBC
    variable = v
    boundary = '0 1 2 3'
  [../]

[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
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
  file_base = ne_deficient_b
  execute_on = 'timestep_end'
[]
