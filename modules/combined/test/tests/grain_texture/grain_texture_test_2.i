# This simulation predicts GB migration of 8 grains and outputs grain texture information
# Mesh adaptivity is not used so that the VectorPostprocessor's output will be uniform
# Time step adaptivity is used
# An AuxVariable is used to calculate the grain boundary locations

[Mesh]
  # Mesh block.  Meshes can be read in or automatically generated
  type = GeneratedMesh
  dim = 3 # Problem dimension
  nx = 10 # Number of elements in the x-direction
  ny = 10 # Number of elements in the y-direction
  nz = 2 # Number of elements in the z-direction
  xmin = 0 # minimum x-coordinate of the mesh
  xmax = 100 # maximum x-coordinate of the mesh
  ymin = 0 # minimum y-coordinate of the mesh
  ymax = 100 # maximum y-coordinate of the mesh
  zmin = 0 # minimum z-coordinate of the mesh
  zmax = 20 # maximum z-coordinate of the mesh
  elem_type = HEX8 # Type of elements used in the mesh
[]

[GlobalParams]
  # Parameters used by several kernels that are defined globally to simplify input file
  op_num = 3 # Number of order parameters used
  var_name_base = gr # Base name of grains
  grain_num = 3 #Number of grains
[]

[Variables]
  # Variable block, where all variables in the simulation are declared
  [./PolycrystalVariables]
  [../]
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    coloring_algorithm = bt
  [../]
  [./grain_tracker]
    type = FauxGrainTracker # Note: FauxGrainTracker only used for testing purposes. Use GrainTracker when using GrainTextureVectorPostprocessor.
    flood_entity_type = ELEMENTAL
    outputs = none
  [../]
  [./euler_angle_file]
    type = EulerAngleFileReader
    file_name = grn_3_rand_2D.tex
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
  [./unique_grains]
    order = CONSTANT
    family = MONOMIAL
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
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    execute_on = timestep_end
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
  [../]
[]

[Materials]
  [./CuGrGr]
    # Material properties
    type = GBEvolution # Quantitative material properties for copper grain growth.  Dimensions are nm and ns
    block = 0 # Block ID (only one block in this problem)
    GBmob0 = 2.5e-6 #Mobility prefactor for Cu from Schonfelder1997
    GBenergy = 0.708 # GB energy in J/m^2
    Q = 0.23 #Activation energy for grain growth from Schonfelder 1997
    T = 450 # K   #Constant temperature of the simulation (for mobility calculation)
    wGB = 14 # nm    #Width of the diffuse GB
  [../]
[]

[VectorPostprocessors]
  [./textureInfo]
    type = GrainTextureVectorPostprocessor
    unique_grains = unique_grains
    euler_angle_provider = euler_angle_file
    sort_by = id # sort output by elem id
  [../]
[]

[Executioner]
  type = Transient # Type of executioner, here it is transient with an adaptive time step
  scheme = bdf2 # Type of time integration (2nd order backward euler), defaults to 1st order backward euler

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  l_max_its = 30 # Max number of linear iterations
  l_tol = 1e-4 # Relative tolerance for linear solves
  nl_max_its = 40 # Max number of nonlinear iterations
  nl_abs_tol = 1e-11 # Relative tolerance for nonlinear solves
  nl_rel_tol = 1e-10 # Absolute tolerance for nonlinear solves
  start_time = 0.0
  num_steps = 1
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
[]
