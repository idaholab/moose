[Mesh]
  type = GeneratedMesh
  dim = 2
  # Note: The following parameters must match the values in “IN100_001_28x28_Marmot.txt” after uniform refine has run
  nx = 7
  ny = 7
  nz = 0
  xmin = 0
  xmax = 7
  ymin = 0
  ymax = 7
  zmin = 0
  zmax = 0
  elem_type = QUAD4
  uniform_refine = 2
[]

[GlobalParams]
  crys_num = 9
  grain_num = 9
  var_name_base = gr
  sd = 3
[]

[UserObjects]
  [./ebsd]
    type = EBSDReader
    filename = 'IN100_001_28x28_Marmot.txt'
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
    #ebsd_reader = ebsd
   [../]
[]

[ICs]
  [./PolycrystalICs]
    [./ReconAuxVarIC]
      ebsd_reader = ebsd
    [../]

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
    order = FIRST
    family = LAGRANGE
  [../]

  [./phi1]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./PHI]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./phi2]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./grn]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./phase]
    order = CONSTANT
    family = MONOMIAL
  [../]

   [./sym]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./rgb]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./op]
    order = CONSTANT
    family = MONOMIAL
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

  [./phi1_aux]
    Aux_param = phi1
    variable = phi1
    type = ReconAuxVarUpdate
    ebsd_reader = ebsd
    execute_on = timestep
  [../]

  [./PHI_aux]
    Aux_param = PHI
    variable = PHI
    type = ReconAuxVarUpdate
    ebsd_reader = ebsd
    execute_on = timestep
  [../]

  [./phi2_aux]
    Aux_param = phi2
    variable = phi2
    type = ReconAuxVarUpdate
    ebsd_reader = ebsd
    execute_on = timestep
  [../]

  [./grn_aux]
    Aux_param = grn
    variable = grn
    type = ReconAuxVarUpdate
    ebsd_reader = ebsd
    execute_on = timestep
  [../]

  [./phase_aux]
    Aux_param = phase
    variable = phase
    type = ReconAuxVarUpdate
    ebsd_reader = ebsd
    execute_on = timestep
  [../]

  [./sym_aux]
    Aux_param = sym
    variable = sym
    type = ReconAuxVarUpdate
    ebsd_reader = ebsd
    execute_on = timestep
  [../]

  [./rgb_aux]
    Aux_param = rgb
    variable = rgb
    type = ReconAuxVarUpdate
    ebsd_reader = ebsd
    execute_on = timestep
  [../]

  [./op_aux]
    Aux_param = op
    variable = op
    type = ReconAuxVarUpdate
    ebsd_reader = ebsd
    execute_on = timestep
  [../].
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

  [./ReconMaterial]
    type = ReconMaterial
    block = 0
    ebsd_reader = ebsd
    GrainTracker_object = grain_tracker
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

 # [./num_grns]
 #   type = NodalFloodCount
 #   variable = bnds
 #   threshold = 0.7
 # [../]
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
  num_steps = 2
  dt = 0.05

  [./Adaptivity]
    initial_adaptivity = 2
    refine_fraction = 0.7
    coarsen_fraction = 0.1
    max_h_level = 3
    # weight_names = 'gr0 gr1 gr2 gr3 gr4 gr5 bnds'
    # weight_values = '0 0 0 0 0 0 1'
  [../]
[]

[Outputs]
  file_base = 1phase_reconstruction_test
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

#[Debug]
#  show_parser = true
#  show_actions = true
#[]



