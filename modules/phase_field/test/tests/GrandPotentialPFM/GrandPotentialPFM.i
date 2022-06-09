# this input file test the implementation of the grand-potential phase-field model based on M.Plapp PRE 84,031601(2011)
# in this simple example, the liquid and solid free energies are parabola with the same curvature and the material properties are constant
# Note that this example also test The SusceptibilityTimeDerivative kernels
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 16
  ny = 16
  xmax = 32
  ymax = 32
[]

[GlobalParams]
  radius = 20.0
  int_width = 4.0
  x1 = 0
  y1 = 0
[]

[Variables]
  [./w]
  [../]
  [./eta]
  [../]
[]

[ICs]
  [./w]
    type = SmoothCircleIC
    variable = w
    # note w = A*(c-cleq), A = 1.0, cleq = 0.0 ,i.e., w = c (in the matrix/liquid phase)
    outvalue = -0.2
    invalue = 0.2
  [../]
  [./eta]
    type = SmoothCircleIC
    variable = eta
    outvalue = 0.0
    invalue = 1.0
  [../]
[]

[Kernels]
  [./w_dot]
    type = SusceptibilityTimeDerivative
    variable = w
    property_name = chi
    variable_names = '' # in this case chi (the susceptibility) is simply a constant
  [../]
  [./Diffusion]
    type = MatDiffusion
    variable = w
    diffusivity = D
    args = ''
  [../]
  [./coupled_etadot]
    type = CoupledSusceptibilityTimeDerivative
    variable = w
    v = eta
    property_name = ft
    variable_names = 'eta'
  [../]
  [./AC_bulk]
    type = AllenCahn
    variable = eta
    property_name = F
    variable_names = 'w'
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

[Materials]
  [./constants]
    type = GenericConstantMaterial
    prop_names  = 'kappa_op  D    L    chi  cseq   cleq   A'
    prop_values = '4.0       1.0  1.0  1.0  0.0  1.0  1.0'
  [../]

  [./liquid_GrandPotential]
    type = DerivativeParsedMaterial
    function = '-0.5 * w^2/A - cleq * w'
    variable_names = 'w'
    property_name = f1
    material_property_names = 'cleq A'
  [../]
  [./solid_GrandPotential]
    type = DerivativeParsedMaterial
    function = '-0.5 * w^2/A - cseq * w'
    variable_names = 'w'
    property_name = f2
    material_property_names = 'cseq A'
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
  [./cs]
    type = DerivativeParsedMaterial
    variable_names = 'w'
    property_name = cs
    material_property_names = 'A cseq'
    function = 'w/A + cseq' # since w = A*(c-cseq)
    derivative_order = 2
  [../]
  [./cl]
    type = DerivativeParsedMaterial
    variable_names = 'w'
    property_name = cl
    material_property_names = 'A cleq'
    function = 'w/A + cleq' # since w = A*(c-cleq)
    derivative_order = 2
  [../]
  [./total_GrandPotential]
    type = DerivativeTwoPhaseMaterial
    variable_names = 'w'
    eta = eta
    fa_name = f1
    fb_name = f2
    derivative_order = 2
    W = 1.0
  [../]
  [./coupled_eta_function]
    type = DerivativeParsedMaterial
    function = '(cs - cl) * dh'
    variable_names = 'eta w'
    property_name = ft
    material_property_names = 'cs cl dh:=D[h,eta]'
    derivative_order = 1
    outputs = exodus
  [../]

  [./concentration]
    type = ParsedMaterial
    property_name = c
    material_property_names = 'dF:=D[F,w]'
    function = '-dF'
    outputs = exodus
  [../]
[]

[Postprocessors]
  [./C]
    type = ElementIntegralMaterialProperty
    mat_prop = c
    execute_on = 'initial timestep_end'
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
  scheme = bdf2
  solve_type = NEWTON

  l_max_its = 15
  l_tol = 1e-3
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8

  num_steps = 5
  dt = 10.0
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = 'TIMESTEP_END'
[]
