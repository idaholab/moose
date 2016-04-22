[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmax = 1000
  ymax = 1000
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 6
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalVoronoiIC]
      rand_seed = 1
      grain_num = 6
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
  [./proc_id]
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
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
  [../]
  [./ghost_elements]
    type = FeatureFloodCountAux
    variable = ghost_elements
    field_display = GHOSTED_ENTITIES
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
  [../]
  [./halos]
    type = FeatureFloodCountAux
    variable = halos
    field_display = HALOS
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    field_display = VARIABLE_COLORING
    execute_on = 'initial timestep_end'
    bubble_object = grain_tracker
  [../]
  [./proc_id]
    type = ProcessorIDAux
    variable = proc_id
    execute_on = 'initial timestep_end'
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./CuGrGr]
    # Material properties
    type = GBEvolution
    block = 0 # Block ID (only one block in this problem)
    T = 500 # Constant temperature of the simulation (for mobility calculation)
    wGB = 90 # Width of the diffuse GB
    GBmob0 = 2.5e-6 # m^4(Js) for copper from Schoenfelder1997
    Q = 0.23 # eV for copper from Schoenfelder1997
    GBenergy = 0.708 # J/m^2 from Schoenfelder1997
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.1
    connecting_threshold = 0.05
    convex_hull_buffer = 5.0
    execute_on = 'initial timestep_end'
    remap_grains = true
    use_single_map = false
    enable_var_coloring = true
    condense_map_info = true
    flood_entity_type = ELEMENTAL
  [../]
  [./DOFs]
    type = NumDOFs
  [../]
[]

[Executioner]
  # !CH2 petsc_options = '-snes -ksp_monitor '
  # scheme = 'bdf2'
  # Preconditioned JFNK (default)
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  l_tol = 1.0e-6
  l_max_its = 30
  nl_rel_tol = 1.0e-10
  nl_max_its = 30
  start_time = 0.0
  num_steps = 100
  dt = 30
[]

[Outputs]
  exodus = true
[]
