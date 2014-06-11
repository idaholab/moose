#
# KKS toy problem in the non-split form
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  nz = 0
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  # order parameter
  [./eta]
    order = THIRD
    family = HERMITE
  [../]

  # hydrogen concentration
  [./c]
    order = THIRD
    family = HERMITE
  [../]

  # hydrogen phase concentration (matrix)
  [./cm]
    order = THIRD
    family = HERMITE
    initial_condition = 0.0
  [../]
  # hydrogen phase concentration (delta phase)
  [./cd]
    order = THIRD
    family = HERMITE
    initial_condition = 0.0
  [../]
[]

[ICs]
  [./eta]
    type = RandomIC
    variable = eta
    min = 0.1
    max = 0.2
    block = 0
  [../]
  [./c]
    type = RandomIC
    variable = c
    min = 0.4
    max = 0.6
    block = 0
  [../]
[]

[BCs]
  [./Periodic]
    [./eta]
      variable = eta
      auto_direction = 'x y'
    [../]
    [./c]
      variable = c
      auto_direction = 'x y'
    [../]
    [./cm]
      variable = cm
      auto_direction = 'x y'
    [../]
    [./cd]
      variable = cd
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  # Free energy of the matrix
  [./fm]
    type = KKSParsedMaterial
    block = 0
    f_name = fm
    args = 'cm'
    function = '(0.1-cm)^2'
    outputs = exodus
  [../]

  # Free energy of the delta phase
  [./fd]
    type = KKSParsedMaterial
    block = 0
    f_name = fd
    args = 'cd'
    function = '(0.9-cd)^2'
    outputs = exodus
  [../]

  # h(eta)
  [./h_eta]
    type = KKSHEtaPolyMaterial
    block = 0
    h_order = HIGH
    eta = eta
    outputs = exodus
  [../]

  # g(eta)
  [./g_eta]
    type = KKSGEtaPolyMaterial
    block = 0
    g_order = SIMPLE
    eta = eta
    outputs = exodus
  [../]

  # constant properties
  [./constants]
    type = GenericConstantMaterial
    block = 0
    prop_names  = 'M   L   kappa constant_zero'
    prop_values = '0.7 0.7 0.4   0            '
  [../]
[]

[Kernels]
  # full transient
  active = 'PhaseConc ChemPotVacancies CHBulk ACBulkF ACBulkC ACInterface dcdt detadt'

  # enforce c = (1-h(eta))*cm + h(eta)*cd
  [./PhaseConc]
    type = KKSPhaseConcentration
    ca       = cm
    variable = cd
    c        = c
    eta      = eta
  [../]

  # enforce pointwise equality of chemical potentials
  [./ChemPotVacancies]
    type = KKSPhaseChemicalPotential
    variable = cm
    cb       = cd
    fa_name  = fm
    fb_name  = fd
  [../]

  #
  # Cahn-Hilliard Equation
  #
  [./CHBulk]
    type = KKSCHBulk
    variable = c
    ca       = cm
    cb       = cd
    fa_name  = fm
    fb_name  = fd
    eta      = eta
  [../]
  [./dcdt]
    type = TimeDerivative
    variable = c
  [../]

  #
  # Allen-Cahn Equation
  #
  [./ACBulkF]
    type = KKSACBulkF
    variable = eta
    fa_name  = fm
    fb_name  = fd
    w        = 0.4
  [../]
  [./ACBulkC]
    type = KKSACBulkC
    variable = eta
    ca       = cm
    cb       = cd
    fa_name  = fm
    fb_name  = fd
  [../]
  [./ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
  [../]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  l_max_its = 100
  nl_max_its = 100

  num_steps = 3

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.1  
  [../]
[]

#
# This still needs finite difference preconditioning as the
# handcoded jacobians are not complete. Check out the split
# solve, which works with SMP preconditioning.
#
[Preconditioning]
  [./mydebug]
    type = FDP
    full = true
  [../]
[]

[Outputs]
  file_base = kks_example
  output_initial = true
  interval = 1
  exodus = true

  [./console]
    type = Console
    perf_log = true
  [../]
[]

