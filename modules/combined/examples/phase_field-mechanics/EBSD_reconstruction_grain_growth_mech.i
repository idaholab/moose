# This example reconstructs the grain structure from an EBSD data file
# Then, an isotropic grain model is run with linear elasticity and an anisotropic
# elasticity tensor that uses the measured EBSD angles.
[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    uniform_refine = 2 #Mesh can go two levels coarser than the EBSD grid
    filename = IN100_128x128.txt
  []
[]

[GlobalParams]
  op_num = 8
  var_name_base = gr
  displacements = 'disp_x disp_y'
[]

[Variables]
  [PolycrystalVariables] #Polycrystal variable generation (30 order parameters)
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [bnds]
  []
  [gt_indices]
    order = CONSTANT
    family = MONOMIAL
  []
  [unique_grains]
    order = CONSTANT
    family = MONOMIAL
  []
  [vonmises_stress]
    order = CONSTANT
    family = MONOMIAL
  []
  [C1111]
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
  [EBSD_grain]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [PolycrystalICs]
    [ReconVarIC]
      ebsd_reader = ebsd
      coloring_algorithm = bt
    []
  []
[]

[Kernels]
  [PolycrystalKernel]
  []
  [PolycrystalElasticDrivingForce]
  []
  [TensorMechanics]
  []
[]

[AuxKernels]
  [BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  []
  [gt_indices]
    type = FeatureFloodCountAux
    variable = gt_indices
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
  []
  [unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
  []
  [C1111]
    type = RankFourAux
    variable = C1111
    rank_four_tensor = elasticity_tensor
    index_l = 0
    index_j = 0
    index_k = 0
    index_i = 0
    execute_on = timestep_end
  []
  [vonmises_stress]
    type = RankTwoScalarAux
    variable = vonmises_stress
    rank_two_tensor = stress
    scalar_type = VonMisesStress
    execute_on = timestep_end
  []
  [phi1]
    type = OutputEulerAngles
    variable = phi1
    euler_angle_provider = ebsd
    grain_tracker = grain_tracker
    output_euler_angle = 'phi1'
    execute_on = 'initial'
  []
  [Phi]
    type = OutputEulerAngles
    variable = Phi
    euler_angle_provider = ebsd
    grain_tracker = grain_tracker
    output_euler_angle = 'Phi'
    execute_on = 'initial'
  []
  [phi2]
    type = OutputEulerAngles
    variable = phi2
    euler_angle_provider = ebsd
    grain_tracker = grain_tracker
    output_euler_angle = 'phi2'
    execute_on = 'initial'
  []
  [grain_aux]
    type = EBSDReaderPointDataAux
    variable = EBSD_grain
    ebsd_reader = ebsd
    data_name = 'feature_id'
    execute_on = 'initial'
  []
[]

[BCs]
  [top_displacement]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = -2.0
  []
  [x_anchor]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0.0
  []
  [y_anchor]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
[]

[Modules]
  [PhaseField]
    [EulerAngles2RGB]
      crystal_structure = cubic
      euler_angle_provider = ebsd
      grain_tracker = grain_tracker
    []
  []
[]

[Materials]
  [Copper]
    # T = 500 # K
    type = GBEvolution
    block = 0
    T = 500
    wGB = 0.6 # um
    GBmob0 = 2.5e-6 # m^4/(Js) from Schoenfelder 1997
    Q = 0.23 # Migration energy in eV
    GBenergy = 0.708 # GB energy in J/m^2
    molar_volume = 7.11e-6; # Molar volume in m^3/mol
    length_scale = 1.0e-6
    time_scale = 1.0e-6
  []
  [ElasticityTensor]
    type = ComputePolycrystalElasticityTensor
    grain_tracker = grain_tracker
  []
  [strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
  []
  [stress]
    type = ComputeLinearElasticStress
    block = 0
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
  [n_elements]
    type = NumElems
    execute_on = 'initial timestep_end'
  []
  [n_nodes]
    type = NumNodes
    execute_on = 'initial timestep_end'
  []
  [DOFs]
    type = NumDOFs
  []
[]

[UserObjects]
  [ebsd]
    type = EBSDReader
  []
  [grain_tracker]
    type = GrainTrackerElasticity
    compute_var_to_feature_map = true
    ebsd_reader = ebsd

    fill_method = symmetric9
    C_ijkl = '1.27e5 0.708e5 0.708e5 1.27e5 0.708e5 1.27e5 0.7355e5 0.7355e5 0.7355e5'
    euler_angle_provider = ebsd
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = '  hypre    boomeramg                   0.7'
  l_tol = 1.0e-4
  l_max_its = 20
  nl_max_its = 20
  nl_rel_tol = 1.0e-8
  start_time = 0.0
  num_steps = 30
  dt = 10
  [Adaptivity]
    initial_adaptivity = 0
    refine_fraction = 0.7
    coarsen_fraction = 0.1
    max_h_level = 2
  []
  [TimeStepper]
    type = IterationAdaptiveDT
    cutback_factor = 0.9
    dt = 10.0
    growth_factor = 1.1
    optimal_iterations = 7
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
