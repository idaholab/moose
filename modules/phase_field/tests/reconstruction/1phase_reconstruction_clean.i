[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 28
  ny = 28
  nz = 0
  xmin = 0
  xmax = 7
  ymin = 0
  ymax = 7
  zmin = 0
  zmax = 0
  elem_type = QUAD4
  #uniform_refine = 2
[]

[GlobalParams]
  op_num = 9
  var_name_base = gr
  sd= 3
[]

[Variables]
  [./ReconstructedVariables]
    EBSD_file_name = IN100_001_28x28_Clean_Marmot.txt
    x1 = 0
    y1 = 0
    z1 = 0
    x2 = 7
    y2 = 7
    z2 = 0
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

#  [./unique_grains]
#    order = FIRST
#    family = LAGRANGE
#  [../]
#
#  [./var_indices]
#    order = FIRST
#    family = LAGRANGE
#  [../]
#
#  [./centroids]
#    order = CONSTANT
#    family = MONOMIAL
#  [../]
[]

[Kernels]
  [./PolycrystalKernel]
  [../]
[]

[AuxKernels]

  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  [../]

#  [./unique_grains]
#    type = NodalFloodCountAux
#    variable = unique_grains
#    execute_on = timestep_end
#    bubble_object = grain_tracker
#    field_display = UNIQUE_REGION
#  [../]

#  [./var_indices]
#    type = NodalFloodCountAux
#    variable = var_indices
#    execute_on = timestep_end
#    bubble_object = grain_tracker
#    field_display = VARIABLE_COLORING
#  [../]

#  [./centroids]
#    type = NodalFloodCountAux
#    variable = centroids
#    execute_on = timestep_end
#    bubble_object = grain_tracker
#    field_display = CENTROID
#  [../]

  [./grn]
    type = GrainIndexAux
    variable = grn
    execute_on = timestep_end
  [../]

  [./phase]
    type = PhaseIndexAux
    variable = phase
    execute_on = timestep_end
  [../]

  [./rgb]
    type = Euler2RGBAux
    variable = rgb
    execute_on = timestep_end
  [../]
[]

#[BCs]
#active = ' '
#   [./Periodic]
#     [./all]
#       auto_direction = 'x y z'
#     [../]
#   [../]
#[]

[Materials]
  [./CuGrGr]
    type = CuGrGrClean
    block = 0
    temp = 500 #K
    wGB = 0.75 #micron
    length_scale = 1.0e-6
    time_scale = 1.0e-6
  [../]

  [./ReconstructionMaterial]
    type = ReconstructionMaterial
    block = 0
    EBSD_file_name = IN100_001_28x28_Clean_Marmot.txt
  [../]
[]

#[Preconditioning]
#  active = ''
#  [./SMP]
#    type = SMP
#    full = true
#  [../]
#[]

[Postprocessors]
#  [./grain_tracker]
#    type = GrainTracker
#    threshold = 0.2
#    connecting_threshold = 0.08
#    convex_hull_buffer = 5.0
#    execute_on = timestep_end
#    remap_grains = true
#    use_single_map = false
#    enable_var_coloring = true
#    condense_map_info = true
#  [../]

  [./DOFs]
    type = NumDOFs
  [../]

  [./n_nodes]
    type = NumNodes
    execute_on = timestep_end
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
  dt = 0.1
[]

 #[./Adaptivity]
 #  initial_adaptivity = 1
 #  refine_fraction = 0.8
 #  coarsen_fraction = 0.1
 #  max_h_level = 3
 #  print_changed_info = true
 #[../]
 # []

[Outputs]
  exodus = true
[]
