[Mesh]
  type = GeneratedMesh
  dim = 2
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
  var_name_base = gr
  sd = 3
[]

[Variables]
  [./ReconstructedVariables]    
    EBSD_file_name = Ti_2Phase_28x28_Sqr_Marmot.txt
    x1 = 0
    y1 = 0
    x2 = 1.4
    y2 = 1.212430
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
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
    type = Euler2RgbAux
    variable = rgb   
    execute_on = timestep
  [../]
[]

[Materials]  
  [./CuGrGr]
    type = CuGrGr
    block = 0
    temp = 500 #K
    wGB = 0.75 #micron
    length_scale = 1.0e-6
    time_scale = 1.0e-4
  [../]

  [./ReconstructionMaterial]
    type = ReconstructionMaterial
    block = 0
    EBSD_file_name = Ti_2Phase_28x28_Sqr_Marmot.txt
  [../]   
[]

[Postprocessors]
  [./n_nodes]
    type = NumNodes
    execute_on = timestep
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
  l_max_its = 15
  nl_rel_tol = 1.0e-9
  nl_max_its = 20
  start_time = 0.0
  num_steps = 3
  dt = 0.1

[./Adaptivity]
   initial_adaptivity = 1
    refine_fraction = 0.7
    coarsen_fraction = 0.1
    max_h_level = 3
  [../]
[]

[Output]
  linear_residuals = true
  file_base = 2phase_reconstruction_test
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
   
    

