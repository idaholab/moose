[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
[]

[Problem]
  kernel_coverage_check = false
[]

[GlobalParams]
  op_num = 4
  var_name_base = gr
[]

[Variables]
  [PolycrystalVariables]
  []
[]

[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    grain_num = 4
    rand_seed = 100
  [../]
  [./dislocation_density_file]
    type = DislocationDensityFileReader
    file_name = test.txt
    lines_to_skip = 0
  [../]
  [./grain_tracker]
    type = GrainTrackerDislocations
    compute_var_to_feature_map = true
    execute_on = 'initial timestep_begin'
    dislocation_density_reader = dislocation_density_file
    polycrystal_ic_uo = voronoi
    tolerate_failure = true
  [../]
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      polycrystal_ic_uo = voronoi
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  nl_abs_tol = 1e-5
[]

[Outputs]
  exodus = true
[]
