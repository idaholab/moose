#
# This test ensures that the equilibrium solution using the dedicated two phase
# formulation is identical to the two order parameters with a Lagrange multiplier
# constraint in lagrange_multiplier.i
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 5
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
    initial_condition = 0.5
  []

  # hydrogen concentration
  [c]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = FunctionIC
      function = x/5
    []
  []

  # chemical potential
  [w]
    order = FIRST
    family = LAGRANGE
  []

  # hydrogen phase concentration (matrix)
  [cm]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.2
  []
  # hydrogen phase concentration (delta phase)
  [cd]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []
[]

[Materials]
  # Free energy of the matrix
  [fm]
    type = DerivativeParsedMaterial
    property_name = fm
    coupled_variables = 'cm'
    expression = '(0.1-cm)^2'
  []

  # Free energy of the delta phase
  [fd]
    type = DerivativeParsedMaterial
    property_name = fd
    coupled_variables = 'cd'
    expression = '(0.9-cd)^2'
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
    prop_names  = 'M   L   kappa'
    prop_values = '0.7 0.7 0.4  '
  []
[]

[Kernels]
  # full transient
  active = 'PhaseConc ChemPotVacancies CHBulk ACBulkF ACBulkC ACInterface dcdt detadt ckernel'

  # enforce c = (1-h(eta))*cm + h(eta)*cd
  [PhaseConc]
    type = KKSPhaseConcentration
    ca = cm
    variable = cd
    c = c
    eta = eta
  []

  # enforce pointwise equality of chemical potentials
  [ChemPotVacancies]
    type = KKSPhaseChemicalPotential
    variable = cm
    cb = cd
    fa_name = fm
    fb_name = fd
  []

  #
  # Cahn-Hilliard Equation
  #
  [CHBulk]
    type = KKSSplitCHCRes
    variable = c
    ca = cm
    fa_name = fm
    w = w
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
    type = KKSACBulkF
    variable = eta
    fa_name = fm
    fb_name = fd
    coupled_variables = 'cm cd'
    w = 0.4
  []
  [ACBulkC]
    type = KKSACBulkC
    variable = eta
    ca = cm
    cb = cd
    fa_name = fm
    mob_name = L
  []
  [ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
    mob_name = L
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
  solve_type = 'PJFNK'
  petsc_options_iname = '-pctype -sub_pc_type -sub_pc_factor_shift_type -pc_factor_shift_type'
  petsc_options_value = ' asm    lu          nonzero                    nonzero'
  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-11

  num_steps = 35
  dt = 10
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

[VectorPostprocessors]
  [c]
    type = LineValueSampler
    variable = c
    start_point = '0 0 0'
    end_point = '5 0 0'
    num_points = 21
    sort_by = x
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
