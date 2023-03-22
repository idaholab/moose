# This test was used for grain grain with re-merging grains based on 
# misorientation angle, and with the initial miscrostructure from
# EBSD data. Distributed Mesh was adopted in FEM.

my_filename = "case2_EBSD"

[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = local_ebsd_Ti.inl
  []
  use_distributed_mesh = true
[]

[GlobalParams]
  op_num = 10
  var_name_base = gr
  length_scale = 1.0e-6
  time_scale = 1.0
[]

[UserObjects]
  [ebsd_reader]
    type = EBSDReader
    custom_columns = 0
  []
  [ebsd]
    type = PolycrystalEBSD
    coloring_algorithm = jp
    ebsd_reader = ebsd_reader
    enable_var_coloring = true
    compute_var_to_feature_map = false
  []
  [grain_tracker]
    type = GrainTrackerMerge
    threshold = 0.5
    connecting_threshold = 0.04
    halo_level = 3
    flood_entity_type = ELEMENTAL
    polycrystal_ic_uo = ebsd
    compute_var_to_feature_map = true

    merge_grains_based_misorientaion = true
    euler_angle_provider = ebsd_reader

    remap_grains = true
  []
  [./term]
    type = Terminator
    expression = 'grain_tracker < 10'
  [../]
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      polycrystal_ic_uo = ebsd
    []
  []
[]

[Variables]
  [PolycrystalVariables]
  []
[]

[AuxVariables]
  [bnds]
  []
  [unique_grains]
    order = CONSTANT
    family = MONOMIAL
  []
  [var_indices]
    order = CONSTANT
    family = MONOMIAL
  []
  [phi1]
    order = CONSTANT
    family = MONOMIAL
  []
  [Phi]
    order = CONSTANT
    family = MONOMIAL
  []
  [phi2]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [PolycrystalKernel]
  []
[]

[AuxKernels]
  [BndsCalc]
    type = BndsCalcAux
    variable = bnds
  []
  [unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
  []
  [var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
  []
  [phi1]
    type = OutputEulerAngles
    variable = phi1
    euler_angle_provider = ebsd_reader
    grain_tracker = grain_tracker
    output_euler_angle = 'phi1'
  []
  [Phi]
    type = OutputEulerAngles
    variable = Phi
    euler_angle_provider = ebsd_reader
    grain_tracker = grain_tracker
    output_euler_angle = 'Phi'
  []
  [phi2]
    type = OutputEulerAngles
    variable = phi2
    euler_angle_provider = ebsd_reader
    grain_tracker = grain_tracker
    output_euler_angle = 'phi2'
  []
[]

[Modules]
  [PhaseField]
    [EulerAngles2RGB]
      # EulerAngle2RGBAction
      # Set up auxvariables and auxkernels to output Euler angles as RGB values interpolated across inverse pole figure
      crystal_structure = hexagonal # hexagonal cubic 
      euler_angle_provider = ebsd_reader
      grain_tracker = grain_tracker
    []
  []
[]

[Materials]
  [./CuGrGr]
    # Material properties
    type = GBEvolution
    T = 973.15 # K
    wGB = 1.0
    GBmob0 = 2.5e-12 #m^4(Js) for copper from Schoenfelder1997
    Q = 0.23
    GBenergy = 0.96
  [../]
[]

[VectorPostprocessors]
  [./grain_volumes] 
    type = FeatureVolumeVectorPostprocessor
    flood_counter = grain_tracker
    output_centroids = true
  [../]
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
  [DOFs]
    type = NumDOFs
  []
  [./run_time]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  [../]
  [./ngrains]
    type = FeatureFloodCount
    variable = bnds
    threshold = 0.7
  [../]
  [./bnd_length]
    type = GrainBoundaryArea
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK

  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre    boomeramg      0.7'

  l_tol = 1.0e-4
  l_max_its = 20
  nl_max_its = 15
  nl_rel_tol = 1.0e-8
  dtmin = 1.0e-4

  start_time = 0.0
  num_steps = 6

  [TimeStepper]
    type = IterationAdaptiveDT
    cutback_factor = 0.8
    dt = 0.1
    growth_factor = 1.1
    optimal_iterations = 7
  []
  [Adaptivity]
    initial_adaptivity = 4
    refine_fraction = 0.8
    coarsen_fraction = 0.3
    max_h_level = 3
  []
[]

[Outputs]
  file_base = ./${my_filename}/out_${my_filename}
  [./my_checkpoint]
    type = Checkpoint
    additional_execute_on = 'FINAL'
  [../]
  [my_exodus]
    type = Nemesis
  [../]
  [./csv]
    type = CSV
  [../]
  print_linear_residuals = false
[]