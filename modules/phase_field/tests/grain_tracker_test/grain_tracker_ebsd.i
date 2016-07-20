[Mesh]
  type = EBSDMesh
  filename = 'test.txt'
[]

[GlobalParams]
  op_num = 7
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
    [../]
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./bnds]
  [../]
  [./unique_grains]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./var_indices]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./ebsd_grains]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./phi1]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Kernels]
  [./PolycrystalKernel]
  [../]
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  [../]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
    field_display = UNIQUE_REGION
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
    field_display = VARIABLE_COLORING
  [../]
  [./grain_aux]
    type = EBSDReaderPointDataAux
    variable = ebsd_grains
    ebsd_reader = ebsd
    data_name = 'grain'
    execute_on = 'initial timestep_end'
  [../]
  [./phi1]
    type = OutputEulerAngles
    euler_angle_provider = ebsd
    output_euler_angle = phi1
    GrainTracker_object = grain_tracker
    variable = phi1
  [../]
[]

[Modules]
  [./PhaseField]
    [./EulerAngles2RGB]
      crystal_structure = cubic
      euler_angle_provider = ebsd
      grain_tracker_object = grain_tracker
    [../]
  [../]
[]

[Materials]
  [./CuGrGr]
    type = GBEvolution
    block = 0
    T = 500 #K
    wGB = 0.75 #micron
    length_scale = 1.0e-6
    time_scale = 1.0e-4

    GBmob0 = 2.5e-6
    Q = 0.23
    GBenergy = 0.708
    molar_volume = 7.11e-6
  [../]
[]

[Postprocessors]
  [./n_nodes]
    type = NumNodes
    execute_on = timestep_end
  [../]

  [./DOFs]
    type = NumDOFs
  [../]

  [./grain_tracker]
    type = GrainTracker
    ebsd_reader = ebsd
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 31 0.7'
  l_tol = 1.0e-4
  l_max_its = 20
  nl_rel_tol = 1.0e-9
  nl_max_its = 20
  start_time = 0.0
  num_steps = 1
  dt = 0.05
[]

[Outputs]
  exodus = true
  print_perf_log = true
[]
