# This simulation predicts GB migration of a 2D copper polycrystal with 100 grains represented with 18 order parameters
# Mesh adaptivity and time step adaptivity are used
# An AuxVariable is used to calculate the grain boundary locations
# Postprocessors are used to record time step and the number of grains

[Mesh]
  # Mesh block.  Meshes can be read in or automatically generated
  type = GeneratedMesh
  dim = 3 # Problem dimension
  nx = 10 # Number of elements in the x-direction
  ny = 10 # Number of elements in the y-direction
  nz = 10
  xmin = 0    # minimum x-coordinate of the mesh
  xmax = 1000 # maximum x-coordinate of the mesh
  ymin = 0    # minimum y-coordinate of the mesh
  ymax = 1000 # maximum y-coordinate of the mesh
  zmin = 0
  zmax = 1000
  uniform_refine = 1 # Initial uniform refinement of the mesh

  parallel_type = distributed
[]

[GlobalParams]
  # Parameters used by several kernels that are defined globally to simplify input file
  op_num = 15 # Number of order parameters used
  var_name_base = gr # Base name of grains
  order = CONSTANT
  family = MONOMIAL
[]

[Variables]
  # Variable block, where all variables in the simulation are declared
  [./PolycrystalVariables]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    grain_num = 25 # Number of grains
    rand_seed = 10
    coloring_algorithm = jp
  [../]
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.2
    connecting_threshold = 0.08
    compute_halo_maps = true # Only necessary for displaying HALOS
    polycrystal_ic_uo = voronoi
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
  # Dependent variables
  [./bnds]
    # Variable used to visualize the grain boundaries in the simulation
    order = FIRST
    family = LAGRANGE
  [../]
  [./unique_grains]
  [../]

  [./var_indices]
  [../]

  [./ghost_regions]
  [../]

  [./halos]
  [../]

  [./halo0]
  [../]

  [./halo1]
  [../]

  [./halo2]
  [../]

  [./halo3]
  [../]

  [./halo4]
  [../]

  [./halo5]
  [../]

  [./halo6]
  [../]

  [./halo7]
  [../]

  [./halo8]
  [../]

  [./halo9]
  [../]

  [./halo10]
  [../]

  [./halo11]
  [../]

  [./halo12]
  [../]

  [./halo13]
  [../]

  [./halo14]
  [../]

  [./proc]
  [../]
[]

[Kernels]
  # Kernel block, where the kernels defining the residual equations are set up.
  [./PolycrystalKernel]
    # Custom action creating all necessary kernels for grain growth.  All input parameters are up in GlobalParams
  [../]
[]

[AuxKernels]
  # AuxKernel block, defining the equations used to calculate the auxvars
  [./bnds_aux]
    # AuxKernel that calculates the GB term
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  [../]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
    execute_on = 'initial timestep_end'
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
    execute_on = 'initial timestep_end'
  [../]
  [./ghosted_entities]
    type = FeatureFloodCountAux
    variable = ghost_regions
    flood_counter = grain_tracker
    field_display = GHOSTED_ENTITIES
    execute_on = 'initial timestep_end'
  [../]
  [./halos]
    type = FeatureFloodCountAux
    variable = halos
    flood_counter = voronoi
    field_display = HALOS
    execute_on = 'initial timestep_end'
  [../]
  [./halo0]
    type = FeatureFloodCountAux
    variable = halo0
    map_index = 0
    field_display = HALOS
    flood_counter = grain_tracker
    execute_on = 'initial timestep_end'
  [../]
  [./halo1]
    type = FeatureFloodCountAux
    variable = halo1
    map_index = 1
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo2]
    type = FeatureFloodCountAux
    variable = halo2
    map_index = 2
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo3]
    type = FeatureFloodCountAux
    variable = halo3
    map_index = 3
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo4]
    type = FeatureFloodCountAux
    variable = halo4
    map_index = 4
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo5]
    type = FeatureFloodCountAux
    variable = halo5
    map_index = 5
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo6]
    type = FeatureFloodCountAux
    variable = halo6
    map_index = 6
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo7]
    type = FeatureFloodCountAux
    variable = halo7
    map_index = 7
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo8]
    type = FeatureFloodCountAux
    variable = halo8
    map_index = 8
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo9]
    type = FeatureFloodCountAux
    variable = halo9
    map_index = 9
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo10]
    type = FeatureFloodCountAux
    variable = halo10
    map_index = 10
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo11]
    type = FeatureFloodCountAux
    variable = halo11
    map_index = 11
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo12]
    type = FeatureFloodCountAux
    variable = halo12
    map_index = 12
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo13]
    type = FeatureFloodCountAux
    variable = halo13
    map_index = 13
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./halo14]
    type = FeatureFloodCountAux
    variable = halo14
    map_index = 14
    field_display = HALOS
    flood_counter = grain_tracker
  [../]
  [./proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = 'initial timestep_end'
  [../]

[]

[Materials]
  [./CuGrGr]
    # Material properties
    type = GBEvolution
    T = 450 # Constant temperature of the simulation (for mobility calculation)
    wGB = 125 # Width of the diffuse GB
    GBmob0 = 2.5e-6 #m^4(Js) for copper from Schoenfelder1997
    Q = 0.23 #eV for copper from Schoenfelder1997
    GBenergy = 0.708 #J/m^2 from Schoenfelder1997
  [../]
[]

[Postprocessors]
  # Scalar postprocessors
  [./dt]
    # Outputs the current time step
    type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient # Type of executioner, here it is transient with an adaptive time step
  scheme = bdf2 # Type of time integration (2nd order backward euler), defaults to 1st order backward euler

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  # Uses newton iteration to solve the problem.
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'asm'

  l_max_its = 30 # Max number of linear iterations
  l_tol = 1e-4 # Relative tolerance for linear solves
  nl_max_its = 40 # Max number of nonlinear iterations
  nl_rel_tol = 1e-10 # Absolute tolerance for nonlienar solves

  start_time = 0.0
  end_time = 4000

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 25 # Initial time step.  In this simulation it changes.
    optimal_iterations = 6 # Time step will adapt to maintain this number of nonlinear iterations
  [../]

#  [./Adaptivity]
#    # Block that turns on mesh adaptivity. Note that mesh will never coarsen beyond initial mesh (before uniform refinement)
##    initial_adaptivity = 2 # Number of times mesh is adapted to initial condition
#    refine_fraction = 0.6 # Fraction of high error that will be refined
#    coarsen_fraction = 0.1 # Fraction of low error that will coarsened
#    max_h_level = 3 # Max number of refinements used, starting from initial mesh (before uniform refinement)
#  [../]
[]

[Outputs]
  exodus = true
  csv = true
  [./pg]
    type = PerfGraphOutput
    execute_on = 'initial final'  # Default is "final"
    level = 2                     # Default is 1
  [../]
[]
