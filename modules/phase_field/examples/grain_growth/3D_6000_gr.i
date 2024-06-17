[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 180
  ny = 180
  nz = 180
  xmin = 0
  xmax = 180
  ymin = 0
  ymax = 180
  zmin = 0
  zmax = 180
  elem_type = HEX8
[]

[GlobalParams]
  op_num = 28
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    grain_num = 6000 # Number of grains
    rand_seed = 8675 # 301
    coloring_algorithm = jp
  [../]
  [./term]
    type = Terminator
    expression = 'grain_tracker < 218'
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalColoringIC]
      polycrystal_ic_uo = voronoi
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [./unique_grains]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ghost_elements]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halos]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./var_indices]
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
    execute_on = 'initial timestep_end'
  [../]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    field_display = UNIQUE_REGION
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
  [../]
  [./ghost_elements]
    type = FeatureFloodCountAux
    variable = ghost_elements
    field_display = GHOSTED_ENTITIES
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
  [../]
  [./halos]
    type = FeatureFloodCountAux
    variable = halos
    field_display = HALOS
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    field_display = VARIABLE_COLORING
    execute_on = 'initial timestep_end'
    flood_counter = grain_tracker
  [../]
[]

#[BCs]
#  [./Periodic]
#    [./All]
#      auto_direction = 'x y'
#    [../]
#  [../]
#[]

[Materials]
  [./Copper]
    type = GBEvolution
    T = 500
    wGB = 3 # um
    GBmob0 = 2.5e-6 #m^4/(Js) from Schoenfelder 1997
    Q = 0.23 #Migration energy in eV
    GBenergy = 0.708 #GB energy in J/m^2
    molar_volume = 7.11e-6 #Molar volume in m^3/mol
    length_scale = 1.0e-6
    time_scale = 1.0
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
  [./n_elements]
    type = NumElements
    execute_on = timestep_end
  [../]
  [./n_nodes]
    type = NumNodes
    execute_on = timestep_end
  [../]
  [./DOFs]
    type = NumDOFs
  [../]
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.1
    compute_halo_maps = true
  [../]
[]

#[Preconditioning]
#  [./SMP]
#    type = SMP
#    full = true
#  [../]
#[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK #Preconditioned JFNK (default)
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'asm'
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1.0e-8
  start_time = 0.0
  num_steps = 500
  dt = 0.0002

  [./TimeStepper]
    type = IterationAdaptiveDT
    cutback_factor = 0.9
    dt = 0.0002
    growth_factor = 1.1
    optimal_iterations = 8
  [../]

  #[./Adaptivity]
  #  initial_adaptivity = 4
  #  refine_fraction = 0.6
  #  coarsen_fraction = 0.1
  #  max_h_level = 4
  #  print_changed_info = true
  #[../]
[]

[Outputs]
  nemesis = true
  checkpoint = true
  csv = true
  [./console]
    type = Console
  [../]
[]
