[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  nz = 0
  xmax = 1000
  ymax = 1000
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  crys_num = 12
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
    ICType = -8
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalVoronoiIC]
      rand_seed = 8675
      grain_num = 12
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [./unique_grains]
    order = FIRST
    family = LAGRANGE
  [../]
  [./var_indices]
    order = FIRST
    family = LAGRANGE
  [../]
  [./centroids]
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
  [../]
  [./unique_grains]
    type = NodalFloodCountAux
    variable = unique_grains
    execute_on = timestep
    bubble_object = grain_tracker
    field_display = UNIQUE_REGION
  [../]
  [./var_indices]
    type = NodalFloodCountAux
    variable = var_indices
    execute_on = timestep
    bubble_object = grain_tracker
    field_display = VARIABLE_COLORING
  [../]
  [./centroids]
    type = NodalFloodCountAux
    variable = centroids
    execute_on = timestep
    bubble_object = grain_tracker
    field_display = CENTROID
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./CuGrGr]
    type = CuGrGr
    block = 0
    temp = 500 # K
    wGB = 70 # nm
  [../]
[]

[Postprocessors]
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.5
    convex_hull_buffer = 5.0
    execute_on = timestep
    remap_grains = true
    use_single_map = false
    enable_var_coloring = true
    condense_map_info = true
    bubble_volume_file = bubble_volumes.txt
  [../]
  [./DOFs]
    type = NumDOFs
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  petsc_options = -snes_mf_operator
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1.0e-9
  start_time = 0.0
  num_steps = 1
  dt = 50.0
[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]

