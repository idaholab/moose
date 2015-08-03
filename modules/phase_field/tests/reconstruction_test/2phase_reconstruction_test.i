[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Mesh]
  type = EBSDMesh
  filename = 'Ti_2Phase_28x28_Sqr_Marmot.txt'
[]

[UserObjects]
  [./ebsd]
    type = EBSDReader
  [../]
[]

[Variables]
  [./c1]
  [../]
  [./c2]
  [../]
[]

[ICs]
  [./phase1_recon]
    type = ReconVarIC
    ebsd_reader = ebsd
    consider_phase = true
    phase = 1
    variable = c1
    all_to_one = true
  [../]
  [./phase2_recon]
    type = ReconVarIC
    ebsd_reader = ebsd
    consider_phase = true
    phase = 2
    variable = c2
    all_to_one = true
  [../]
[]

[AuxVariables]
  [./PHI1]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./PHI]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./PHI2]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./PHASE]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./phi1_aux]
    type = TestEBSDAux
    variable = PHI1
    ebsd_reader = ebsd
    data_name = 'phi1'
    execute_on = 'initial'
  [../]
  [./phi_aux]
    type = TestEBSDAux
    variable = PHI
    ebsd_reader = ebsd
    data_name = 'phi'
    execute_on = 'initial'
  [../]
  [./phi2_aux]
    type = TestEBSDAux
    variable = PHI2
    ebsd_reader = ebsd
    data_name = 'phi2'
    execute_on = 'initial'
  [../]
  [./phase_aux]
    type = TestEBSDAux
    variable = PHASE
    ebsd_reader = ebsd
    data_name = 'phase'
    execute_on = 'initial'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  output_initial = true
  interval = 1
  exodus = true
  print_perf_log = true
[]
