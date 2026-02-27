[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = 'ebsd_9.txt'
  []
[]

[GlobalParams]
  op_num = 4
  var_name_base = gr
[]

[Variables]
  [PolycrystalVariables]
  []
[]

[Kernels]
  [PolycrystalKernel]
  []
[]

[AuxVariables]
[phi1]
family = MONOMIAL
order = CONSTANT
[]
[Phi]
family = MONOMIAL
order = CONSTANT
[]
[phi2]
family = MONOMIAL
order = CONSTANT
[]
  [crystrot_00]
    family = MONOMIAL
    order = CONSTANT
  []
  [crystrot_01]
    family = MONOMIAL
    order = CONSTANT
  []
  [crystrot_02]
    family = MONOMIAL
    order = CONSTANT
  []
  [crystrot_11]
    family = MONOMIAL
    order = CONSTANT
  []
  [crystrot_12]
    family = MONOMIAL
    order = CONSTANT
  []
  [crystrot_22]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
[phi1]
    type = OutputEulerAngles
    euler_angle_provider = ebsd_reader
    output_euler_angle = phi1
    grain_tracker = grain_tracker
    variable = phi1
  []
  [Phi]
    type = OutputEulerAngles
    euler_angle_provider = ebsd_reader
    output_euler_angle = Phi
    grain_tracker = grain_tracker
    variable = Phi
  []
  [phi2]
    type = OutputEulerAngles
    euler_angle_provider = ebsd_reader
    output_euler_angle = phi2
    grain_tracker = grain_tracker
    variable = phi2
  []
  [aux_crystrot_00]
    type = MaterialRankTwoTensorAux
    i = 0
    j = 0
    property = crysrot
    variable = crystrot_00
  []
  [aux_crystrot_01]
    type = MaterialRankTwoTensorAux
    i = 0
    j = 1
    property = crysrot
    variable = crystrot_01
  []
  [aux_crystrot_02]
    type = MaterialRankTwoTensorAux
    i = 0
    j = 2
    property = crysrot
    variable = crystrot_02
  []
  [aux_crystrot_11]
    type = MaterialRankTwoTensorAux
    i = 1
    j = 1
    property = crysrot
    variable = crystrot_11
  []
  [aux_crystrot_12]
    type = MaterialRankTwoTensorAux
    i = 1
    j = 2
    property = crysrot
    variable = crystrot_12
  []
  [aux_crystrot_22]
    type = MaterialRankTwoTensorAux
    i = 2
    j = 2
    property = crysrot
    variable = crystrot_22
  []
[]

[UserObjects]
  [ebsd_reader]
    type = EBSDReader
  []
  [ebsd]
    type = PolycrystalEBSD
    coloring_algorithm = bt
    ebsd_reader = ebsd_reader
    output_adjacency_matrix = true
  []
  [grain_tracker]
  type = GrainTrackerElasticity
  connecting_threshold = 0.05
  compute_var_to_feature_map = true
  flood_entity_type = elemental
  execute_on = 'initial timestep_begin'

  euler_angle_provider = ebsd_reader
  fill_method = symmetric9
  C_ijkl = '1.27e5 0.708e5 0.708e5 1.27e5 0.708e5 1.27e5 0.7355e5 0.7355e5 0.7355e5'

  outputs = none
  []
[]

[Materials]
  [ElasticityTensor]
    type = ComputePolycrystalElasticityTensor
    grain_tracker = grain_tracker
    euler_angle_provider = ebsd_reader
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      polycrystal_ic_uo = ebsd
    []
  []
[]

[Materials]
  [CuGrGr]
    type = GBEvolution
    T = 500 #K
    wGB = 0.75 #micron
    length_scale = 1.0e-6
    time_scale = 1.0e-4

    GBmob0 = 2.5e-6
    Q = 0.23
    GBenergy = 0.708
    molar_volume = 7.11e-6
  []
[]

[Postprocessors]
  [check_euler_angle_1]
    type = ElementAverageValue
    variable = phi1
  []
  [check_euler_angle_2]
    type = ElementAverageValue
    variable = Phi
  []
  [check_euler_angle_3]
    type = ElementAverageValue
    variable = phi2
  []
  [check_rotation_00]
    type = ElementAverageValue
    variable = crystrot_00
  []
  [check_rotation_01]
    type = ElementAverageValue
    variable = crystrot_01
  []
  [check_rotation_02]
    type = ElementAverageValue
    variable = crystrot_02
  []
  [check_rotation_11]
    type = ElementAverageValue
    variable = crystrot_11
  []
  [check_rotation_12]
    type = ElementAverageValue
    variable = crystrot_12
  []
  [check_rotation_22]
    type = ElementAverageValue
    variable = crystrot_22
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
                        '-pc_hypre_boomeramg_strong_threshold'
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
  csv = true
[]
