# checks that OrientedBoxMarker behaves as desired
[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -6
  xmax = 4
  nx = 10
  ymin = -2
  ymax = 10
  ny = 12
  zmin = -5
  zmax = 7
  nz = 12
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./timed]
    type = TimeDerivative
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  end_time = 1
[]

[Adaptivity]
  marker = obm
  [./Markers]
    [./obm]
      type = OrientedBoxMarker
      centre = '-1 4 1'
      width = 5
      length = 10
      height = 4
      width_direction = '2 1 0'
      length_direction = '-1 2 2'
      inside = refine
      outside = do_nothing
    [../]
  [../]
[]

[Output]
  file_base = obm
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
   
    
