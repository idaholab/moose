[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 1000
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 4
  grain_num = 4
  var_name_base = gr
    # the stored energy is a fixed free energy shift for each grain
    # here we lower the energy of the center grain in the cell relative
    # to all other grains. This causes that grain to grow at the expense
    # of the other grains.
  stored_energy = '0.11 0.12 0 0.13'
  grain_tracker = faux_tracker
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalHexGrainIC]
      x_offset = .5
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [./energy]
    #order = CONSTANT
    #family = MONOMIAL
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./PolycrystalKernel]
    penalty = 1
  [../]
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  [../]
  [./StoredEnergy]
    type = PolyStoredEnergyAux
    variable = energy
    execute_on = timestep_end
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
  [./Copper]
    type = GBEvolution
    block = 0
    T = 500 # K
    wGB = 60 # nm
    GBmob0 = 2.5e-6 #m^4/(Js) from Schoenfelder 1997
    Q = 0.23 #Migration energy in eV
    GBenergy = 0.708 #GB energy in J/m^2
  [../]
[]

[Postprocessors]
  active = ''
  [./ngrains]
    type = FeatureFloodCount
    variable = bnds
    threshold = 0.7
  [../]
[]

[UserObjects]
  [./faux_tracker]
    # we use a FauxGrainTracker as the number of grains is equal to the
    # number of order parameters used. The stored energy kernel uses the
    # GrainTrackerInterface internally to look up the grain identity for the
    # current order parameter.
    type = FauxGrainTracker
  [../]
[]

[Preconditioning]
  active = ''
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  # petsc_options_iname = '-pc_type'
  # petsc_options_value = 'lu'
  type = Transient
  scheme = 'bdf2'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1.0e-9
  start_time = 0.0
  num_steps = 40
  dt = 10.0
[]

[Outputs]
  print_linear_residuals = false
  interval = 2
  exodus = true
[]
