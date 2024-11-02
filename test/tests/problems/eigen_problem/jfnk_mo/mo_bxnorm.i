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

  [./T]
    order = FIRST
    family = LAGRANGE
    # this will make T also scaled by normalization
    eigen = true
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

  [./diff_T]
    type = Diffusion
    variable = T
  [../]
  [./src_T]
    type = CoupledForce
    variable = T
    v = u
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]

  [./eigenU]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  [../]

  [./homogeneousT]
    type = DirichletBC
    variable = T
    boundary = '0 1 2 3'
    value = 0
  [../]
[]

[Problem]
  type = EigenProblem
  bx_norm = uint
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNKMO
  nl_rel_tol = 1e-6
#  normalization = uint
#  normal_factor = 10
[]

[Postprocessors]
  [uint]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'initial linear'
  []
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = 'timestep_end'
[]
