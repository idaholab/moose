[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 1000
  zmin = 0
  zmax = 0
  elem_type = QUAD4
  uniform_refine = 2
[]

[Variables]
  # order parameter
  [./eta]
    order = FIRST
    family = LAGRANGE
  [../]

  # real vacancy concentration
  [./cv]
    order = FIRST
    family = LAGRANGE
  [../]

  # real gas concentration
  [./cg]
    order = FIRST
    family = LAGRANGE
  [../]


  # bubble phase vacancy concentration
  [./cbv]
    order = THIRD
    family = HERMITE
  [../]

  # bubble phase gas concentration
  [./cbg]
    order = THIRD
    family = HERMITE
  [../]


  # solid phase vacancy concentration
  [./csv]
    order = THIRD
    family = HERMITE
  [../]

  # solid phase gas concentration
  [./csg]
    order = THIRD
    family = HERMITE
  [../]
[]

[Materials]
  # bubble phase free energy
  [./FreeEnergyBubble]
    type = KKSParsedMaterial
    block = 0
    args = 'cbv cbg'
    constant_names  = 'T   kB'
    constant_values = '400 8.15e-5'
    function = kB*T*(cbv*log(cbv)+cbg^3)
    f_base = Fb
  [../]

  # solid phase free energy
  [./FreeEnergySolid]
    type = KKSParsedMaterial
    block = 0
    args = 'csv csg'
    constant_names  = 'T   kB'
    constant_values = '400 8.15e-5'
    function = kB*T*(csv*log(csv)+csg^3)
    f_base = Fs
  [../]
[]

[Kernels]
  [./ConcentrationVacancies]
    type = KKSPhaseConcentration
    ca       = cbv
    variable = csv
    c        = cv
    eta      = eta
  [../]

  [./ConcentrationGas]
    type = KKSPhaseConcentration
    ca       = cbg
    variable = csg
    c        = cg
    eta      = eta
  [../]

  [./ChemPotVacancies]
    type = KKSPhaseChemicalPotential
    variable = cbv
    cb       = csv
    fa_base  = Fb
    fb_base  = Fs
  [../]

  [./ChemPotGas]
    type = KKSPhaseChemicalPotential
    variable = cbg
    cb       = csg
    fa_base  = Fb
    fb_base  = Fs
  [../]

[]

[Executioner]
  type = Transient
  scheme = crank-nicolson

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'
  l_max_its = 30
  nl_max_its = 20
  start_time = 0.0
  num_steps = 2
  dt = 50.0
[]

[Outputs]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]

