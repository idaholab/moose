# this input file test the implementation of the grand-potential phase-field model based on M.Plapp PRE 84,031601(2011)
# in this simple example, the liquid and solid free energies are parabola with the same curvature and the material properties are constant
# Note that this example also test The SusceptibilityTimeDerivative kernels
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 16
  ny = 16
  nz = 0
  xmin = 0
  xmax = 64
  ymin = 0
  ymax = 64
  zmin = 0
  zmax = 0
[]
[GlobalParams]
  int_width = 4.0
  block = 0
[]

[Variables]
  [./w]
  [../]
  [./eta]
  [../]
[]

[AuxVariables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./IC_w]
    x1 = 32
    y1 = 32
    radius = 20.0
    outvalue = 0.1
    variable = w # note w = A*(c-cleq), A = 1.0, cleq = 0.0 ,i.e., w = c (in the matrix/liquid phase)
    invalue = 0.0
    type = SmoothCircleIC
  [../]
  [./IC_eta]
    x1 = 32
    y1 = 32
    radius = 20.0
    outvalue = 0.0
    variable = eta
    invalue = 1.0
    type = SmoothCircleIC
  [../]
[]

[BCs]
 [./Periodic]
  [./all]
   auto_direction = 'x y'
  [../]
 [../]
[]

[Kernels]
  [./w_dot]
    type = SusceptibilityTimeDerivative
    variable = w
    f_name = chi
    args = '' # in this case chi (the susceptibility) is simply a constant
  [../]
  [./Diffusion]
    type = MatDiffusion
    variable = w
    D_name = D
    args = ''
  [../]
  [./coupled_etadot]
    type = CoupledSusceptibilityTimeDerivative
    variable = w
    v = eta
    f_name = ft
    args = 'eta'
  [../]
  [./AC_bulk]
    type = AllenCahn
    variable = eta
    f_name = F
    args = 'w'
  [../]
  [./AC_int]
    type = ACInterface
    variable = eta
  [../]
  [./e_dot]
    type = TimeDerivative
    variable = eta
  [../]
[]

[AuxKernels]
  # Calculate the concentration
  [./Actual_c]
    type = ParsedAux
    variable = c
    args = 'w eta'
    constant_names = 'A cs cl'
    constant_expressions = '1.0 1.0 0.0'
    function = '(w/A+cs*eta^3*(6.0*eta^2-15.0*eta+10.0)+cl*(1.0-eta^3*(6.0*eta^2-15.0*eta+10.0)))'
    #g(eta)=eta^3*(6.0*eta^2-15.0*eta+10.0) is an interpolation function
  [../]
[]

[Materials]
  [./Liquid_GrandPoten]
    type = DerivativeParsedMaterial
    function = '(-0.5*w^2/A-cl*w)'
    args = 'w'
    f_name = f1
    constant_names = 'A cl'
    constant_expressions = '1.0 0.0'
  [../]
  [./Solid_GrandPoten]
    type = DerivativeParsedMaterial
    function = '(-0.5*w^2/A-cs*w)'
    args = 'w'
    f_name = f2
    constant_names = 'A cs'
    constant_expressions = '1.0 1.0'
  [../]
  [./switching_function]
    type = SwitchingFunctionMaterial
    eta = eta
    h_order = HIGH
  [../]
  [./barrier_function]
    type = BarrierFunctionMaterial
    eta = eta
  [../]
  [./total_free_en]
    type = DerivativeTwoPhaseMaterial
    block = 0
    args = 'w'
    eta = eta
    fa_name = f1
    fb_name = f2
    derivative_order = 2
    W = 1.0
  [../]
  [./const]
    type = GenericConstantMaterial
    prop_names = 'kappa_op D L chi'
    prop_values = '4.0 1.0 1.0 1.0'
  [../]
  [./CoupledEtaFn]
    type = DerivativeParsedMaterial
    function = '(deltac*30.0*eta^2*(eta^2-2.0*eta+1.0))'
    args = 'eta'
    f_name = ft
    constant_names = 'cs cl deltac'
    constant_expressions = '1.0 0.0 cs-cl'
    derivative_order = 1
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  nl_max_its = 15
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'asm'
  l_max_its = 15
  l_tol = 1.0e-3
  nl_rel_tol = 1.0e-8
  start_time = 0.0
  num_steps = 1
  nl_abs_tol = 1e-8
  [./TimeStepper]
    type = ConstantDT
    dt = 1.0
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  interval = 1
  print_perf_log = true
[]
