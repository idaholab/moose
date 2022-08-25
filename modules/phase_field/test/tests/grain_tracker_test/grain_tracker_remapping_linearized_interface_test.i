# This simulation predicts GB migration of a 2D copper polycrystal with 100 grains represented with 18 order parameters
# Mesh adaptivity and time step adaptivity are used
# An AuxVariable is used to calculate the grain boundary locations
# Postprocessors are used to record time step and the number of grains

[Mesh]
  # Mesh block.  Meshes can be read in or automatically generated
  type = GeneratedMesh
  dim = 2 # Problem dimension
  nx = 25 # Number of elements in the x-direction
  ny = 25 # Number of elements in the y-direction
  xmax = 1000 # maximum x-coordinate of the mesh
  ymax = 1000 # maximum y-coordinate of the mesh
[]

[GlobalParams]
  # Parameters used by several kernels that are defined globally to simplify input file
  op_num = 8 # Number of order parameters used
  var_name_base = psi # Base name of grains
  bound_value = 5 # +/- bound value
[]

[Modules]
  [PhaseField]
    [GrainGrowthLinearizedInterface]
      op_name_base = gr
      mobility = L
      kappa = kappa_op
    []
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      polycrystal_ic_uo = voronoi
      linearized_interface = true
    []
  []
[]

[UserObjects]
  [voronoi]
    type = PolycrystalVoronoi
    grain_num = 10 # Number of grains
    rand_seed = 13405
    int_width = 100
  []
  [grain_tracker]
    type = GrainTracker
    threshold = -4
  []
[]

[Materials]
  [CuGrGr]
    # Material properties
    type = GBEvolution
    T = 450 # Constant temperature of the simulation (for mobility calculation)
    wGB = 100 # Width of the diffuse GB
    GBmob0 = 2.5e-6 # m^4(Js) for copper from Schoenfelder1997
    Q = 0.23 # eV for copper from Schoenfelder1997
    GBenergy = 0.708 # J/m^2 from Schoenfelder1997
  []
[]

[Executioner]
  # Uses newton iteration to solve the problem.
  type = Transient # Type of executioner, here it is transient with an adaptive time step
  scheme = bdf2 # Type of time integration (2nd order backward euler), defaults to 1st order backward euler

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -snes_type'
  petsc_options_value = 'hypre    boomeramg      vinewtonrsls'

  l_max_its = 30 # Max number of linear iterations
  l_tol = 1e-4 # Relative tolerance for linear solves
  nl_max_its = 13 # Max number of nonlinear iterations

  num_steps = 7
  dt = 100
[]

[Outputs]
  csv = true
[]
