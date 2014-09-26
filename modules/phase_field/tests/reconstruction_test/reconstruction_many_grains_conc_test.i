[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Mesh]
  type = EBSDMesh
  filename = UO2-524_205_205_10_FFT_Marmot_Slice16.txt
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

[Variables]
  [./PolycrystalVariables]
  [../]
  [./c]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./ReconVarIC]
      ebsd_reader = ebsd
      consider_phase = true
      phase = 1
    [../]
  [../]
  [./c_IC]
    variable = c
    ebsd_reader = ebsd
    consider_phase = true
    type = ReconVarIC
    all_to_one = true
    phase = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  file_base = many_grains_conc
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]

