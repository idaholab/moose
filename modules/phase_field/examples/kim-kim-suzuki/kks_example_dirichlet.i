#
# KKS simple example in the split form
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD4
  nx = 50
  ny = 2
  nz = 0
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 0.4
  zmin = 0
  zmax = 0
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

  # hydrogen concentration
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]

  # chemical potential
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]

  # Liquid phase solute concentration
  [./cl]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
  # Solid phase solute concentration
  [./cs]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.9
  [../]
[]

[Functions]
  [./ic_func_eta]
    type = ParsedFunction
    expression = 0.5*(1.0-tanh((x)/sqrt(2.0)))
  [../]
  [./ic_func_c]
    type = ParsedFunction
    expression = '0.9*(0.5*(1.0-tanh(x/sqrt(2.0))))^3*(6*(0.5*(1.0-tanh(x/sqrt(2.0))))^2-15*(0.5*(1.0-tanh(x/sqrt(2.0))))+10)+0.1*(1-(0.5*(1.0-tanh(x/sqrt(2.0))))^3*(6*(0.5*(1.0-tanh(x/sqrt(2.0))))^2-15*(0.5*(1.0-tanh(x/sqrt(2.0))))+10))'
  [../]
[]

[ICs]
  [./eta]
    variable = eta
    type = FunctionIC
    function = ic_func_eta
  [../]
  [./c]
    variable = c
    type = FunctionIC
    function = ic_func_c
  [../]
[]

[BCs]
  [./left_c]
    type = DirichletBC
    variable = 'c'
    boundary = 'left'
    value = 0.5
  [../]
  [./left_eta]
    type = DirichletBC
    variable = 'eta'
    boundary = 'left'
    value = 0.5
  [../]
[]

[Materials]
  # Free energy of the liquid
  [./fl]
    type = DerivativeParsedMaterial
    property_name = fl
    coupled_variables = 'cl'
    expression = '(0.1-cl)^2'
  [../]

  # Free energy of the solid
  [./fs]
    type = DerivativeParsedMaterial
    property_name = fs
    coupled_variables = 'cs'
    expression = '(0.9-cs)^2'
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
  # enforce c = (1-h(eta))*cl + h(eta)*cs
  [./PhaseConc]
    type = KKSPhaseConcentration
    ca       = cl
    variable = cs
    c        = c
    eta      = eta
  [../]

  # enforce pointwise equality of chemical potentials
  [./ChemPotSolute]
    type = KKSPhaseChemicalPotential
    variable = cl
    cb       = cs
    fa_name  = fl
    fb_name  = fs
  [../]

  #
  # Cahn-Hilliard Equation
  #
  [./CHBulk]
    type = KKSSplitCHCRes
    variable = c
    ca       = cl
    fa_name  = fl
    w        = w
  [../]

  [./dcdt]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
  [./ckernel]
    type = SplitCHWRes
    mob_name = M
    variable = w
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
    coupled_variables = 'cl cs'
  [../]
  [./ACBulkC]
    type = KKSACBulkC
    variable = eta
    ca       = cl
    cb       = cs
    fa_name  = fl
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
  nl_abs_tol = 1e-10

  end_time = 800
  dt = 4.0
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

[Postprocessors]
  [./dofs]
    type = NumDOFs
  [../]
  [./integral]
    type = ElementL2Error
    variable = eta
    function = ic_func_eta
  [../]
[]

[Outputs]
  exodus = true
  console = true
  gnuplot = true
[]
