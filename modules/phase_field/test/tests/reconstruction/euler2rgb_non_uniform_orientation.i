[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = ebsd_scan.txt
  []
[]

[GlobalParams]
  op_num = 10
  var_name_base = gr
[]

[UserObjects]
  [ebsd_reader]
    type = EBSDReader
    bins = 40
  []
  [ebsd]
    type = PolycrystalEBSD
    coloring_algorithm = jp
    ebsd_reader = ebsd_reader
    enable_var_coloring = true
  []
  [grain_tracker]
    type = GrainTracker
    flood_entity_type = ELEMENTAL
    compute_halo_maps = true # For displaying HALO fields
    polycrystal_ic_uo = ebsd
  []
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
[]

[Kernels]
  [PolycrystalKernel]
  []
[]

[AuxKernels]
  [BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  []
[]

[Modules]
  [PhaseField]
    [EulerAngles2RGB]
      crystal_structure = cubic
      euler_angle_provider = ebsd_reader
      grain_tracker = grain_tracker
    []
  []
[]

[Materials]
  [Copper]
    # T = 500 # K
    type = GBEvolution
    T = 500
    wGB = 0.6 # um
    GBmob0 = 2.5e-6 # m^4/(Js) from Schoenfelder 1997
    Q = 0.23 # Migration energy in eV
    GBenergy = 0.708 # GB energy in J/m^2
    molar_volume = 7.11e-6 # Molar volume in m^3/mol
    length_scale = 1.0e-6
    time_scale = 1.0e-6
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

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = Newton
  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre    boomeramg      0.7'
  l_tol = 1.0e-6
  l_max_its = 100
  nl_max_its = 20
  nl_rel_tol = 1.0e-8
  start_time = 0.0
  num_steps = 0
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
