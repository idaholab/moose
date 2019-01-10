[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
[]

[Variables]
  [./u]
  [../]
[]

[Adaptivity]
  [./Markers]
    [./box]
      type = BoxMarker
      bottom_left = '0.6 0.7 0'
      top_right = '0.9 0.9 0'
      inside = refine
      outside = do_nothing
    [../]
  [../]
  marker = box
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[VectorPostprocessors]
  [./mem]
    type = VectorMemoryUsage
    execute_on = 'INITIAL TIMESTEP_END NONLINEAR LINEAR'
    report_peak_value = true
    mem_units = kilobytes # or bytes, megabytes, gigabytes
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  csv = true
[]
