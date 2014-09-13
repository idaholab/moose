[Mesh]
  type = EBSDMesh
  filename = 'UO2-524_205_205_10_FFT_Marmot_Slice16.txt'
[]

[GlobalParams]
  op_num = 10
  grain_num = 581
  var_name_base = gr
 # sd = 3
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
    [../]
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[Kernels]
  [./PolycrystalKernel]
  [../]
[]

[Materials]
  [./CuGrGr]
    type = CuGrGr
    block = 0
    T = 500 #K
    wGB = 6.0 #micron
    length_scale = 1.0e-6
    time_scale = 1.0e-4
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  file_base = many_grains
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
