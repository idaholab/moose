[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
  uniform_refine = 4
[]

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
  [./ddt_u]
    type = TimeDerivative
    variable = u
  [../]

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./ddt_v]
    type = TimeDerivative
    variable = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[DiracKernels]
  [./nonlinear_source]
    type = NonlinearSource
    variable = u
    coupled_var = v
    scale_factor = 1000
    point = '0.2 0.3 0'
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 1
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  [../]
[]

[Preconditioning]
  [./precond]
    type = SMP
    # 'full = true' is required for computeOffDiagJacobian() to get
    # called.  If you comment this out, you should see that this test
    # requires more linear and nonlinear iterations.
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 5
  dt = 1
  l_max_its = 100
  nl_max_its = 6
  dtmin = 1.e-3
  nl_abs_tol = 1.e-13
[]

[Postprocessors]
  # A PointValue postprocessor at the Dirac point location
  [./point_value]
    type = PointValue
    variable = u
    point = '0.2 0.3 0'
  [../]
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
