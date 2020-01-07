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
    type = MassEigenKernel
    variable = u
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

[Executioner]
  type = NonlinearEigen

  bx_norm = 'unorm'

  free_power_iterations = 2
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-50
  k0 = 1.0
  output_after_power_iterations = false

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
  active = 'unorm udiff'

  [./unorm]
    type = ElementIntegralVariablePostprocessor
    variable = u
    # execute on residual is important for nonlinear eigen solver!
    execute_on = linear
  [../]

  [./udiff]
    type = ElementL2Diff
    variable = u
    outputs = console
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = ne_coupled
  exodus = true
[]
