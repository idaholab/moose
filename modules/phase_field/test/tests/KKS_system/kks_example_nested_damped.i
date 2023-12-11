#
# Two-phase damped nested KKS with log-free energies
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  nz = 0
  xmin = -2.5
  xmax = 2.5
  ymin = -2.5
  ymax = 2.5
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[AuxVariables]
  [Fglobal]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Variables]
  # order parameter
  [eta]
    order = FIRST
    family = LAGRANGE
  []

  # hydrogen concentration
  [c]
    order = FIRST
    family = LAGRANGE
  []

  # chemical potential
  [w]
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  [eta]
    variable = eta
    type = SmoothCircleIC
    x1 = 0.0
    y1 = 0.0
    radius = 1.5
    invalue = 1
    outvalue = 0.0
    int_width = 0.75
  []
  [c]
    variable = c
    type = SmoothCircleIC
    x1 = 0.0
    y1 = 0.0
    radius = 1.5
    invalue = 0.9
    outvalue = 0.1
    int_width = 0.75
  []
[]

[BCs]
  [Periodic]
    [all]
      variable = 'eta w c'
      auto_direction = 'x y'
    []
  []
[]

[Materials]
  # Free energy of the matrix
  [fm]
    type = DerivativeParsedMaterial
    property_name = fm
    expression = 'cm*log(cm/1e-4) + (1-cm)*log((1-cm)/(1-1e-4))'
    material_property_names = 'cm'
    additional_derivative_symbols = 'cm'
    compute = false
  []

  # Free energy of the delta phase
  [fd]
    type = DerivativeParsedMaterial
    property_name = fd
    expression = 'cd*log(cd/0.9999) + (1-cd)*log((1-cd)/(1-0.9999))'
    material_property_names = 'cd'
    additional_derivative_symbols = 'cd'
    compute = false
  []
  [C]
    type = DerivativeParsedMaterial
    property_name = 'C'
    material_property_names = 'cm cd'
    expression = '(cm>0)&(cm<1)&(cd>0)&(cd<1)'
    compute = false
  []
  # Compute phase concentrations
  [PhaseConcentrationMaterial]
    type = KKSPhaseConcentrationMaterial
    global_cs = 'c'
    ci_names = 'cm cd'
    ci_IC = '0.1 0.9'
    fa_name = fm
    fb_name = fd
    h_name = h
    min_iterations = 1
    max_iterations = 100
    absolute_tolerance = 1e-15
    relative_tolerance = 1e-8
    step_size_tolerance = 1e-05
    nested_iterations = iter
    outputs = exodus
    damped_Newton = true
    conditions = C
    damping_factor = 0.8
  []

  # Compute chain rule terms
  [PhaseConcentrationDerivatives]
    type = KKSPhaseConcentrationDerivatives
    global_cs = 'c'
    eta = eta
    ci_names = 'cm cd'
    fa_name = fm
    fb_name = fd
    h_name = h
  []

  # h(eta)
  [h_eta]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    eta = eta
  []

  # g(eta)
  [g_eta]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta
  []

  # constant properties
  [constants]
    type = GenericConstantMaterial
    prop_names = 'M   L   kappa'
    prop_values = '0.7 0.7 0.4  '
  []
[]

[Kernels]
  # full transient
  active = 'CHBulk ACBulkF ACBulkC ACInterface dcdt detadt ckernel'

  #
  # Cahn-Hilliard Equation
  #
  [CHBulk]
    type = NestedKKSSplitCHCRes
    variable = c
    global_cs = 'c'
    w = w
    all_etas = eta
    ca_names = 'cm cd'
    fa_name = fm
    coupled_variables = 'eta w'
  []
  [dcdt]
    type = CoupledTimeDerivative
    variable = w
    v = c
  []
  [ckernel]
    type = SplitCHWRes
    mob_name = M
    variable = w
  []

  #
  # Allen-Cahn Equation
  #
  [ACBulkF]
    type = NestedKKSACBulkF
    variable = eta
    global_cs = 'c'
    ci_names = 'cm cd'
    fa_name = fm
    fb_name = fd
    g_name = g
    h_name = h
    mob_name = L
    w = 0.4
    coupled_variables = 'c'
  []
  [ACBulkC]
    type = NestedKKSACBulkC
    variable = eta
    global_cs = 'c'
    ci_names = 'cm cd'
    fa_name = fm
    h_name = h
    mob_name = L
    coupled_variables = 'c'
  []
  [ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
  []
  [detadt]
    type = TimeDerivative
    variable = eta
  []
[]

[AuxKernels]
  [GlobalFreeEnergy]
    variable = Fglobal
    type = KKSGlobalFreeEnergy
    fa_name = fm
    fb_name = fd
    w = 0.4
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
    solve_type = 'PJFNK'

    petsc_options_iname = '-pctype -sub_pc_type -sub_pc_factor_shift_type -pc_factor_shift_type'
    petsc_options_value = ' asm    lu          nonzero                    nonzero'

  l_max_its = 100
  nl_max_its = 100
  num_steps = 3

  dt = 1e-5
[]

#
# Precondition using handcoded off-diagonal terms
#
[Preconditioning]
  [full]
    type = SMP
    full = true
  []
[]

[Outputs]
  file_base = kks_example_nested_damped
  exodus = true
[]
