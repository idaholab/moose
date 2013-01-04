# [Postprocessors]
# [./ave_gr_area]
# type = NodalFloodCount
# variable = bnds
# threshold = 0.7
# [../]
# []

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  nz = 0
  xmax = 500
  ymax = 500
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  crys_num = 12
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
    x1 = 0.0
    y1 = 0.0
    x2 = 500.0
    y2 = 500.0
    periodic = '1 1 0'
    grain_num = 12
    rand_seed = 8675
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [./grain_map]
    order = FIRST
    family = LAGRANGE
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
  [./mapper]
    type = NodalFloodCountAux
    variable = grain_map
    execute_on = timestep
    bubble_object = grains
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
    wGB = 50 # nm
  [../]
[]

[UserObjects]
  [./grains]
    type = GrainTracker
#    type = NodalFloodCount
    threshold = 0.9
    tracking_step = 1
    execute_on = timestep

    variable = 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7 gr8 gr9 gr10 gr11'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  petsc_options = '-snes_mf_operator'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1.0e-9
  start_time = 0.0
  num_steps = 5
  dt = 15.0
[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]

