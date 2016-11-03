#
# In this test we set the initial condition of two variables
# based on solely the phase information in a given EBSD data file,
# ignoring the feature IDs entirely
#

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Mesh]
  type = EBSDMesh
  filename = 'Ti_2Phase_28x28_ebsd.txt'
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
    type = ReconPhaseVarIC
    ebsd_reader = ebsd
    phase = 1
    variable = c1
  [../]
  [./phase2_recon]
    type = ReconPhaseVarIC
    ebsd_reader = ebsd
    phase = 2
    variable = c2
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
  [./APHI2]
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
    type = EBSDReaderPointDataAux
    variable = PHI1
    ebsd_reader = ebsd
    data_name = 'phi1'
    execute_on = 'initial'
  [../]
  [./phi_aux]
    type = EBSDReaderPointDataAux
    variable = PHI
    ebsd_reader = ebsd
    data_name = 'phi'
    execute_on = 'initial'
  [../]
  [./phi2_aux]
    type = EBSDReaderPointDataAux
    variable = PHI2
    ebsd_reader = ebsd
    data_name = 'phi2'
    execute_on = 'initial'
  [../]
  [./phase_aux]
    type = EBSDReaderPointDataAux
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
  exodus = true
[]
