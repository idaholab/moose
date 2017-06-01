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

 uniform_refine = 0
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./T]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./power]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = DiffMKernel
    variable = u
    mat_prop = diffusion
    offset = 0.0
  [../]

  [./rhs]
    type = Reaction
    variable = u
    eigen_kernel = true
  [../]

  [./diff_T]
    type = Diffusion
    variable = T
  [../]
  [./src_T]
    type = CoupledForce
    variable = T
    v = power
  [../]
[]

[AuxKernels]
  [./power_ak]
    type = NormalizationAux
    variable = power
    source_variable = u
    normalization = unorm
    # this coefficient will affect the eigenvalue.
    normal_factor = 10
    execute_on = linear
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]

  [./homogeneousT]
    type = DirichletBC
    variable = T
    boundary = '0 1 2 3'
    value = 0
  [../]
[]

[Materials]
  [./dc]
    type = VarCouplingMaterial
    var = T
    block = 0
    base = 1.0
    coef = 1.0
  [../]
[]

[Problem]
  type = EigenProblem
  eigen_problem_type = gen_non_hermitian
  n_eigen_pairs = 1
  n_basis_vectors = 18
  which_eigen_pairs = TARGET_MAGNITUDE
[]


[Executioner]
  type = Steady
  eigen_solve_type = MONOLITH_NEWTON
  petsc_options_iname = '-eps_power_snes_mf_operator'
  petsc_options_value = '1'
[]

[Postprocessors]
  [./unorm]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = linear
  [../]
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  csv = true
  file_base = ne_coupled
  execute_on = 'timestep_end'
  [./console]
    type = Console
    outlier_variable_norms = false
  [../]
[]
