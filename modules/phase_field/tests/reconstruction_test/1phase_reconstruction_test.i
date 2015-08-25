[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Mesh]
  type = EBSDMesh
  filename = IN100_001_28x28_Marmot.txt
[]

[GlobalParams]
  op_num = 9
  var_name_base = gr
[]

[UserObjects]
  [./ebsd]
    type = EBSDReader
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./ReconVarIC]
      ebsd_reader = ebsd
      consider_phase = false
    [../]
  [../]
[]

[Variables]
  [./PolycrystalVariables]
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
  [./GRAIN]
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
  [./grain_aux]
    type = TestEBSDAux
    variable = GRAIN
    ebsd_reader = ebsd
    data_name = 'grain'
    execute_on = 'initial'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  interval = 1
  exodus = true
[]
