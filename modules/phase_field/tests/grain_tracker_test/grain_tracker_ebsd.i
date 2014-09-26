[Mesh]
  type = EBSDMesh
  filename = 'test.txt'
[]

[GlobalParams]
  op_num = 9
  grain_num = 9
  var_name_base = gr
  sd = 3
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
  [./bnds]
  [../]
  [./unique_grains]
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
    execute_on = timestep
  [../]
  [./unique_grains]
    type = NodalFloodCountAux
    variable = unique_grains
    execute_on = timestep
    bubble_object = grain_tracker
    field_display = UNIQUE_REGION
  [../]
[]

[Materials]
  [./CuGrGr]
    type = CuGrGr
    block = 0
    T = 500 #K
    wGB = 0.75 #micron
    length_scale = 1.0e-6
    time_scale = 1.0e-4
  [../]
[]

[Postprocessors]
  [./n_nodes]
    type = NumNodes
    execute_on = timestep
  [../]

  [./DOFs]
    type = NumDOFs
  [../]

   [./grain_tracker]
     type = GrainTracker
     threshold = 0.2
     convex_hull_buffer = 1.0
     use_single_map = false
     enable_var_coloring = true
     condense_map_info = true
     connecting_threshold = 0.2
     compute_op_maps = true
     execute_on = timestep_begin
     tracking_step = 0
     remap_grains = false
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
  output_initial = true
  interval = 1
  exodus = true

  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
    nonlinear_residuals = true
  [../]
[]



