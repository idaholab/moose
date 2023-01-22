[Mesh]
 type = GeneratedMesh
 dim = 2
 xmin = 0
 xmax = 100
 ymin = 0
 ymax = 100
 elem_type = QUAD4
 nx = 8
 ny = 8

 uniform_refine = 0

 displacements = 'x_disp y_disp'
[]

#The minimum eigenvalue for this problem is 2*(pi/a)^2 + 2 with a = 100.
#Its inverse will be 0.49950700634518.

[Variables]
  active = 'u'

  [./u]
    # second order is way better than first order
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./x_disp]
  [../]
  [./y_disp]
  [../]
[]

[AuxKernels]
  [./x_disp]
    type = FunctionAux
    variable = x_disp
    function = x_disp_func
  [../]
  [./y_disp]
    type = FunctionAux
    variable = y_disp
    function = y_disp_func
  [../]
[]

[Functions]
  [./x_disp_func]
    type = ParsedFunction
    expression = 0
  [../]
  [./y_disp_func]
    type = ParsedFunction
    expression = 0
  [../]
[]

[Kernels]
  active = 'diff rea rhs'

  [./diff]
    type = Diffusion
    variable = u
    use_displaced_mesh = true
  [../]

  [./rea]
    type = CoefReaction
    variable = u
    coefficient = 2.0
    use_displaced_mesh = true
  [../]

  [./rhs]
    type = MassEigenKernel
    variable = u
    use_displaced_mesh = true
  [../]
[]

[BCs]
  active = 'homogeneous'

  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
    use_displaced_mesh = true
  [../]
[]

[Executioner]
  type = InversePowerMethod

  min_power_iterations = 11
  max_power_iterations = 400
  Chebyshev_acceleration_on = true
  eig_check_tol = 1e-12
  k0 = 0.5

  bx_norm = 'unorm'
  xdiff = 'udiff'
  normalization = 'unorm'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
  active = 'unorm udiff'

  [./unorm]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = linear
    use_displaced_mesh = true
  [../]

  [./udiff]
    type = ElementL2Diff
    variable = u
    execute_on = 'linear timestep_end'
    use_displaced_mesh = true
  [../]
[]

[Outputs]
  file_base = ipm
  exodus = true
  hide = 'x_disp y_disp'
[]
