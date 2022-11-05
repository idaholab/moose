#
# This test validates the phase concentration calculation for the KKS system
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  nz = 0
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

# We set c and eta...
[BCs]
  # (and ca for debugging purposes)

  [./left]
    type = DirichletBC
    variable = c
    boundary = 'left'
    value = 0.1
  [../]
  [./right]
    type = DirichletBC
    variable = c
    boundary = 'right'
    value = 0.9
  [../]
  [./top]
    type = DirichletBC
    variable = eta
    boundary = 'top'
    value = 0.1
  [../]
  [./bottom]
    type = DirichletBC
    variable = eta
    boundary = 'bottom'
    value = 0.9
  [../]
[]

[Variables]
  # concentration
  [./c]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  [../]

  # order parameter
  [./eta]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]

  # phase concentration a
  [./ca]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.2
  [../]

  # phase concentration b
  [./cb]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.3
  [../]
[]

[Materials]
  # simple toy free energy
  [./fa]
    type = DerivativeParsedMaterial
    property_name = Fa
    coupled_variables = 'ca'
    expression = 'ca^2'
  [../]
  [./fb]
    type = DerivativeParsedMaterial
    property_name = Fb
    coupled_variables = 'cb'
    expression = '(1-cb)^2'
  [../]

  # h(eta)
  [./h_eta]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    eta = eta
    outputs = exodus
  [../]
[]

[Kernels]
  active = 'cdiff etadiff phaseconcentration chempot'
  ##active = 'cbdiff cdiff etadiff chempot'
  #active = 'cadiff cdiff etadiff phaseconcentration'
  ##active = 'cadiff cbdiff cdiff etadiff'

  [./cadiff]
    type = Diffusion
    variable = ca
  [../]

  [./cbdiff]
    type = Diffusion
    variable = cb
  [../]

  [./cdiff]
    type = Diffusion
    variable = c
  [../]

  [./etadiff]
    type = Diffusion
    variable = eta
  [../]

  # ...and solve for ca and cb
  [./phaseconcentration]
    type = KKSPhaseConcentration
    ca       = ca
    variable = cb
    c        = c
    eta      = eta
  [../]

  [./chempot]
    type = KKSPhaseChemicalPotential
    variable = ca
    cb       = cb
    fa_name  = Fa
    fb_namee  = Fb
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  #solve_type = 'NEWTON'
  petsc_options_iname = '-pctype -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = ' asm    lu          nonzero'
[]

[Preconditioning]
  active = 'full'
  #active = 'mydebug'
  #active = ''
  [./full]
    type = SMP
    full = true
  [../]
  [./mydebug]
    type = FDP
    full = true
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = kks_phase_concentration
  exodus = true
[]
