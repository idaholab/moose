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
  [./left]
    type = DirichletBC
    variable = c
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = c
    boundary = 'right'
    value = 1
  [../]
  [./top]
    type = DirichletBC
    variable = eta
    boundary = 'top'
    value = 0
  [../]
  [./bottom]
    type = DirichletBC
    variable = eta
    boundary = 'bottom'
    value = 1
  [../]
[]

[Variables]
  # concentration 
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]

  # order parameter
  [./eta]
    order = FIRST
    family = LAGRANGE
  [../]
  
  # phase concentration a
  [./ca]
    order = FIRST
    family = LAGRANGE
  [../]

  # phase concentration b
  [./cb]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Materials]
  # simple toy free energy 
  [./fa]
    type = KKSParsedMaterial
    block = 0
    f_base = Fa
    args = 'ca'
    function = 'ca^2'
  [../]
  [./fb]
    type = KKSParsedMaterial
    block = 0
    f_base = Fb
    args = 'cb'
    function = '(1-cb)^2'
  [../]
  
  # h(eta)
  [./h_eta]
    type = KKSHEtaPolyMaterial
    block = 0
    h_order = HIGH
    eta = eta
    outputs = exodus
  [../]
[]

[Kernels]
  [./cdiff]
    type = Diffusion
    variable = c
  [../]

  [./etadiff]
    type = Diffusion
    variable = eta
  [../]

  # ...and solve for ca and cb
  [./ConcentrationVacancies]
    type = KKSPhaseConcentration
    ca       = ca
    variable = cb
    c        = c
    eta      = eta
  [../]
  [./ChemPotVacancies]
    type = KKSPhaseChemicalPotential
    variable = ca
    cb       = cb
    fa_base  = Fa
    fb_base  = Fb
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = kks_phase_concentration
  output_initial = false
  interval = 1
  exodus = true

  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]

