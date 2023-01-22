#
# KKS ternary (3 chemical component) system example in the split form
# We track c1 and c2 only, since c1 + c2 + c3 = 1
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 150
  ny = 15
  nz = 0
  xmin = -25
  xmax = 25
  ymin = -2.5
  ymax = 2.5
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[AuxVariables]
  [./Fglobal]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Variables]
  # order parameter
  [./eta]
    order = FIRST
    family = LAGRANGE
  [../]

  # solute 1 concentration
  [./c1]
    order = FIRST
    family = LAGRANGE
  [../]

  # solute 2 concentration
  [./c2]
    order = FIRST
    family = LAGRANGE
  [../]


  # chemical potential solute 1
  [./w1]
    order = FIRST
    family = LAGRANGE
  [../]

  # chemical potential solute 2
  [./w2]
    order = FIRST
    family = LAGRANGE
  [../]


  # Liquid phase solute 1 concentration
  [./c1l]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]

  # Liquid phase solute 2 concentration
  [./c2l]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.05
  [../]

  # Solid phase solute 1 concentration
  [./c1s]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.8
  [../]

  # Solid phase solute 2 concentration
  [./c2s]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]

[]

[Functions]
  [./ic_func_eta]
    type = ParsedFunction
    expression = '0.5*(1.0-tanh((x)/sqrt(2.0)))'
  [../]
  [./ic_func_c1]
    type = ParsedFunction
    expression = '0.8*(0.5*(1.0-tanh(x/sqrt(2.0))))^3*(6*(0.5*(1.0-tanh(x/sqrt(2.0))))^2-15*(0.5*(1.0-tanh(x/sqrt(2.0))))+10)+0.1*(1-(0.5*(1.0-tanh(x/sqrt(2.0))))^3*(6*(0.5*(1.0-tanh(x/sqrt(2.0))))^2-15*(0.5*(1.0-tanh(x/sqrt(2.0))))+10))'
  [../]
  [./ic_func_c2]
    type = ParsedFunction
    expression = '0.1*(0.5*(1.0-tanh(x/sqrt(2.0))))^3*(6*(0.5*(1.0-tanh(x/sqrt(2.0))))^2-15*(0.5*(1.0-tanh(x/sqrt(2.0))))+10)+0.05*(1-(0.5*(1.0-tanh(x/sqrt(2.0))))^3*(6*(0.5*(1.0-tanh(x/sqrt(2.0))))^2-15*(0.5*(1.0-tanh(x/sqrt(2.0))))+10))'
  [../]

[]


[ICs]
  [./eta]
    variable = eta
    type = FunctionIC
    function = ic_func_eta
  [../]
  [./c1]
    variable = c1
    type = FunctionIC
    function = ic_func_c1
  [../]
  [./c2]
    variable = c2
    type = FunctionIC
    function = ic_func_c2
  [../]

[]

[Materials]
  # Free energy of the liquid
  [./fl]
    type = DerivativeParsedMaterial
    property_name = fl
    coupled_variables = 'c1l c2l'
    expression = '(0.1-c1l)^2+(0.05-c2l)^2'
  [../]

  # Free energy of the solid
  [./fs]
    type = DerivativeParsedMaterial
    property_name = fs
    coupled_variables = 'c1s c2s'
    expression = '(0.8-c1s)^2+(0.1-c2s)^2'
  [../]

  # h(eta)
  [./h_eta]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    eta = eta
  [../]

  # g(eta)
  [./g_eta]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta
  [../]

  # constant properties
  [./constants]
    type = GenericConstantMaterial
    prop_names  = 'M   L   eps_sq'
    prop_values = '0.7 0.7 1.0  '
  [../]
[]

[Kernels]
  # enforce c1 = (1-h(eta))*c1l + h(eta)*c1s
  [./PhaseConc1]
    type = KKSPhaseConcentration
    ca       = c1l
    variable = c1s
    c        = c1
    eta      = eta
  [../]

  # enforce c2 = (1-h(eta))*c2l + h(eta)*c2s
  [./PhaseConc2]
    type = KKSPhaseConcentration
    ca       = c2l
    variable = c2s
    c        = c2
    eta      = eta
  [../]

  # enforce pointwise equality of chemical potentials
  [./ChemPotSolute1]
    type = KKSPhaseChemicalPotential
    variable = c1l
    cb       = c1s
    fa_name  = fl
    fb_name  = fs
    args_a   = 'c2l'
    args_b   = 'c2s'
  [../]
  [./ChemPotSolute2]
    type = KKSPhaseChemicalPotential
    variable = c2l
    cb       = c2s
    fa_name  = fl
    fb_name  = fs
    args_a   = 'c1l'
    args_b   = 'c1s'

  [../]

  #
  # Cahn-Hilliard Equations
  #
  [./CHBulk1]
    type = KKSSplitCHCRes
    variable = c1
    ca       = c1l
    fa_name  = fl
    w        = w1
    args_a   = 'c2l'
  [../]
  [./CHBulk2]
    type = KKSSplitCHCRes
    variable = c2
    ca       = c2l
    fa_name  = fl
    w        = w2
    args_a   = 'c1l'
  [../]

  [./dc1dt]
    type = CoupledTimeDerivative
    variable = w1
    v = c1
  [../]
  [./dc2dt]
    type = CoupledTimeDerivative
    variable = w2
    v = c2
  [../]

  [./w1kernel]
    type = SplitCHWRes
    mob_name = M
    variable = w1
  [../]
  [./w2kernel]
    type = SplitCHWRes
    mob_name = M
    variable = w2
  [../]


  #
  # Allen-Cahn Equation
  #
  [./ACBulkF]
    type = KKSACBulkF
    variable = eta
    fa_name  = fl
    fb_name  = fs
    w        = 1.0
    coupled_variables = 'c1l c1s c2l c2s'
  [../]
  [./ACBulkC1]
    type = KKSACBulkC
    variable = eta
    ca       = c1l
    cb       = c1s
    fa_name  = fl
    coupled_variables     = 'c2l'
  [../]
  [./ACBulkC2]
    type = KKSACBulkC
    variable = eta
    ca       = c2l
    cb       = c2s
    fa_name  = fl
    coupled_variables     = 'c1l'
  [../]
  [./ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = eps_sq
  [../]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[AuxKernels]
  [./GlobalFreeEnergy]
    variable = Fglobal
    type = KKSGlobalFreeEnergy
    fa_name = fl
    fb_name = fs
    w = 1.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      ilu          nonzero'

  l_max_its = 100
  nl_max_its = 100

  num_steps = 50
  dt = 0.1
[]

#
# Precondition using handcoded off-diagonal terms
#
[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  exodus = true
[]
