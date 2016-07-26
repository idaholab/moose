[Mesh]
  type = EBSDMesh
  filename = UO2-SMALL.txt
[]

[GlobalParams]
  op_num = 10
  var_name_base = gr
[]

[ICs]
  [./PolycrystalICs]
    [./ReconVarIC]
      ebsd_reader = ebsd
      phase = 2
    [../]
  [../]
  [./void_phase]
    type = ReconPhaseVarIC
    variable = c
    ebsd_reader = ebsd
    phase = 1
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./c]
  [../]
  [./bnds]
  [../]
  [./ebsd_numbers]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Kernels]
  [./PolycrystalKernel]
    c = c
  [../]
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  [../]
  [./ebsd_numbers]
    type = EBSDReaderAvgDataAux
    data_name = feature_id
    ebsd_reader = ebsd
    grain_tracker = grain_tracker
    variable = ebsd_numbers
    execute_on = 'initial timestep_end'
  [../]
[]

[Modules]
  [./PhaseField]
    [./EulerAngles2RGB]
      crystal_structure = cubic
      grain_tracker = grain_tracker
      euler_angle_provider = ebsd
      no_grain_color = '.1 .1 .1'
    [../]
  [../]
[]

[Materials]
  [./UO2]
    type = GBEvolution
    block = 0
    T = 2273
    wGB = 10.0
    GBenergy = 1.58
    GBmob0 = 9.2124e-9
    Q = 2.77
    length_scale = 1.0e-6
    time_scale = 60.0
  [../]
[]

[Postprocessors]
  [./grain_tracker]
    type = GrainTracker
    execute_on = 'initial timestep_begin'
    ebsd_reader = ebsd
  [../]
[]

[UserObjects]
  [./ebsd]
    type = EBSDReader
    execute_on = initial
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_grmres_restart '
  petsc_options_value = '   asm        lu            1               21'
  start_time = 0.0
  dt = 0.2
  num_steps = 1
[]

[Outputs]
  exodus = true
  execute_on = 'INITIAL TIMESTEP_END'
  print_perf_log = true
[]
