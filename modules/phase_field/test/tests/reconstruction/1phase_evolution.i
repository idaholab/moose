#
# In this test we set the initial condition of a set of order parameters
# by pulling out the grain data from given EBSD data file ignoring the phase completely.
#

[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = IN100_001_28x28_Marmot.txt
  []
[]

[GlobalParams]
  op_num = 5
  var_name_base = gr
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
    type = GrainTracker
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

[Kernels]
  [PolycrystalKernel]
  []
[]

[AuxVariables]
  [feature]
    family = MONOMIAL
    order = CONSTANT
  []
  [bnds]
  []
[]

[AuxKernels]
  [feature]
    type = EBSDReaderAvgDataAux
    variable = feature
    ebsd_reader = ebsd_reader
    grain_tracker = grain_tracker
    data_name = feature_id
    execute_on = 'initial timestep_end'
  []
  [bnds]
    type = BndsCalcAux
    variable = bnds
    execute_on = 'initial timestep_end'
  []
[]

[Materials]
  [CuGrGr]
    # Material properties
    type = GBEvolution # Quantitative material properties for copper grain growth.  Dimensions are nm and ns
    block = 0 # Block ID (only one block in this problem)
    GBmob0 = 2.5e-6 #Mobility prefactor for Cu from Schonfelder1997
    GBenergy = 0.708 # GB energy in J/m^2
    Q = 0.23 #Activation energy for grain growth from Schonfelder 1997
    T = 500 # K   #Constant temperature of the simulation (for mobility calculation)
    wGB = 1 # nm    #Width of the diffuse GB
    #outputs = exodus
    length_scale = 1e-06
    time_scale = 1e-6
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 10
[]

[Outputs]
  exodus = true
[]
