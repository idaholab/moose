#
# This test is for the 3-phase KKS model
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

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Variables]
  # concentration
  [./c]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  [../]

  # order parameter 1
  [./eta1]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]

  # order parameter 2
  [./eta2]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.9
  [../]

  # order parameter 3
  [./eta3]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  [../]

  # phase concentration 1
  [./c1]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.2
  [../]

  # phase concentration 2
  [./c2]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  [../]

  # phase concentration 3
  [./c3]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.8
  [../]
[]

[Materials]
  # simple toy free energies
  [./f1]
    type = DerivativeParsedMaterial
    block = 0
    f_name = F1
    args = 'c1'
    function = '(c1-0.2)^2'
  [../]
  [./f2]
    type = DerivativeParsedMaterial
    block = 0
    f_name = F2
    args = 'c2'
    function = '(c2-0.5)^2'
  [../]
  [./f3]
    type = DerivativeParsedMaterial
    block = 0
    f_name = F3
    args = 'c3'
    function = '(c3-0.8)^2'
  [../]

  # Switching functions for each phase
  # h1(eta1, eta2, eta3)
  [./h1_eta]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta1
    eta_j = eta2
    eta_k = eta3
    f_name = h1
  [../]
  # h2(eta1, eta2, eta3)
  [./h2_eta]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta3
    eta_j = eta1
    eta_k = eta2
    f_name = h2
  [../]
  # h3(eta1, eta2, eta3)
  [./h3_eta]
    type = SwitchingFunction3PhaseMaterial
    eta_i = eta2
    eta_j = eta3
    eta_k = eta1
    f_name = h3
  [../]
[]

[Kernels]
  [./cdiff]
    type = Diffusion
    variable = c
  [../]

  [./eta1diff]
    type = Diffusion
    variable = eta1
  [../]

  [./eta2diff]
    type = Diffusion
    variable = eta2
  [../]

  # Kernels for constraint equation eta1 + eta2 + eta3 = 1
  # eta3 is the nonlinear variable for the constraint equation
  [./eta3reaction]
    type = MatReaction
    variable = eta3
    mob_name = 1
  [../]
  [./eta1reaction]
    type = MatReaction
    variable = eta3
    v = eta1
    mob_name = 1
  [../]
  [./eta2reaction]
    type = MatReaction
    variable = eta3
    v = eta2
    mob_name = 1
  [../]
  [./one]
    type = BodyForce
    variable = eta3
    value = -1.0
  [../]

  # Phase concentration constraints
  [./chempot12]
    type = KKSPhaseChemicalPotential
    variable = c1
    cb       = c2
    fa_name  = F1
    fb_name  = F2
  [../]
  [./chempot23]
    type = KKSPhaseChemicalPotential
    variable = c2
    cb       = c3
    fa_name  = F2
    fb_name  = F3
  [../]
  [./phaseconcentration]
    type = KKSMultiPhaseConcentration
    variable = c3
    cj = 'c1 c2 c3'
    hj_names = 'h1 h2 h3'
    etas = 'eta1 eta2 eta3'
    c = c
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  #solve_type = 'NEWTON'
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
  exodus = true
[]
