# This simulation tests the TricrystalTripleJunctionIC

[Mesh]
  # Mesh block. Meshes can be read in or automatically generated.
  type = GeneratedMesh
  dim = 2 # Problem dimension
  nx = 11 # Number of elements in the x direction
  ny = 11 # Number of elements in the y direction
  xmax = 1001 # Maximum x-coordinate of mesh
  xmin = 0 # Minimum x-coordinate of mesh
  ymax = 1001 # Maximum y-coordinate of mesh
  ymin = 0 # Minimum y-coordinate of mesh
  elem_type = QUAD4 # Type of elements used in the mesh
  uniform_refine = 3
[]

[GlobalParams]
  # Parameters used by several kernels that are defined globally to simplify input file
  op_num = 3 # Number of order parameters used
  var_name_base = gr # base name of grains
  v = 'gr0 gr1 gr2' # Names of the grains
  theta1 = 135 # Angle the first grain makes at the triple junction
  theta2 = 100 # Angle the second grain makes at the triple junction
  length_scale = 1.0e-9 # Length scale in nm
  time_scale = 1.0e-9 # Time scale in ns
[]

[Variables]
  [./gr0]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
       type = TricrystalTripleJunctionIC
       op_index = 1
    [../]
  [../]

  [./gr1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
       type = TricrystalTripleJunctionIC
       op_index = 2
    [../]
  [../]

  [./gr2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
       type = TricrystalTripleJunctionIC
       op_index = 3
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
    # Variable used to visualize the grain boundaries in the simulation
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  # Kernels block where the kernels defining the residual equations are set up
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
[]

[Materials]
  [./material]
    # Material properties
    type = GBEvolution
    T = 450 # Constant temperature of the simulation (for mobility calculation)
    wGB = 14 # Width of the diffuse GB
    GBmob0 = 2.5e-6 #m^4(Js) for copper from Schoenfelder1997
    Q = 0.23 #eV for copper from Schoenfelder1997
    GBenergy = 0.708 #J/m^2 from Schoenfelder1997
  [../]
[]

[Postprocessors]
  # Scalar postprocessors
  [./grain_tracker]
    type = FauxGrainTracker
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true # Outputs to the Exodus file format
  execute_on = 'final'
[]

[Problem]
  solve = false
[]
