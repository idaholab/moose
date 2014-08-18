[Mesh]
  type = GeneratedMesh
  dim = 2
  # Note: The following parameters must match the values in “Ti_2Phase_28x28_Sqr_Marmot.txt” after uniform refine has run.
  nx = 7
  ny = 7
  nz = 0
  xmin = 0
  xmax = 1.4
  ymin = 0
  ymax = 1.212430
  zmin = 0
  zmax = 0
  elem_type = QUAD4
  uniform_refine = 2
[]

[GlobalParams]
  crys_num = 4
  grain_num = 4
  var_name_base = eta
  sd = 3
[]

[UserObjects]
  [./ebsd]
    type = EBSDReader
    filename = 'Ti_2Phase_28x28_Sqr_Marmot.txt'
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./ReconstructionIC]
    ebsd_reader = ebsd
    [../]
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]

  [./phi1]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./PHI]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./phi2]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./grn]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./phase]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./rgb]
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
    execute_on = timestep
  [../]

  [./phi1_aux]
    type = TestEBSDAux
    variable = phi1
    ebsd_reader = ebsd
    data_name = 'PHI1'
    execute_on = initial
  [../]

  [./phi_aux]
    type = TestEBSDAux
    variable = PHI
    ebsd_reader = ebsd
    data_name = 'PHI'
    execute_on = initial
  [../]
  [./phi2_aux]
    type = TestEBSDAux
    variable = phi2
    ebsd_reader = ebsd
    data_name = 'PHI2'
    execute_on = initial
  [../]

  [./grain_aux]
    type = TestEBSDAux
    variable = grn
    ebsd_reader = ebsd
    data_name = 'GRAIN'
    execute_on = initial
  [../]

  [./grn]
    type = GrainIndexAux
    variable = grn
    execute_on = timestep
  [../]

  [./phase]
    type = PhaseIndexAux
    variable = phase
    execute_on = timestep
  [../]

  [./rgb]
    type = Euler2RGBAux
    variable = rgb
    execute_on = timestep
  [../]
[]

[Materials]
  [./CuGrGr]
    type = CuGrGr
    block = 0
    T = 500 #K
    wGB = 0.15 #micron
    length_scale = 1.0e-6
    time_scale = 1.0e-4
  [../]

  [./ReconstructionMaterial]
    type = ReconstructionMaterial
    block = 0
    ebsd_reader = ebsd
  [../]
[]

[Postprocessors]
  [./n_nodes]
    type = NumNodes
    execute_on = timestep
  [../]

[./DOFs]
    type = NumDOFs
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 31 0.7'
  l_tol = 1.0e-4
  l_max_its = 20
  nl_rel_tol = 1.0e-9
  nl_max_its = 20
  start_time = 0.0
  num_steps = 2
  dt = 0.05

[./Adaptivity]
   initial_adaptivity = 1
    refine_fraction = 0.7
    coarsen_fraction = 0.1
    max_h_level = 3
  [../]
[]

[Outputs]
  file_base = 2phase_reconstruction_test
  output_initial = true
  interval = 1
  exodus = true

  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
    nonlinear_residuals = true
  [../]
[]

[Debug]
  show_parser = true
  show_actions = true
[]


