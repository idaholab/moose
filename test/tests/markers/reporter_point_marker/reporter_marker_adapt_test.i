[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 6
  dt = 0.1
[]

[Reporters]
  [coords]
    type=ConstantReporter
    real_vector_names = 'y z'
    real_vector_values = '.51 .91; 0 0'
    outputs=none
  []
[]
[Functions]
  [xfcn]
    type = ParsedFunction
    expression = t+0.01 #offset so marker is not on element edge
  []
[]

[Postprocessors]
  [xfcn_pp]
    type = FunctionValuePostprocessor
    function = xfcn
    execute_on = timestep_end
    outputs = none
  []
  [x_pp]
    type = Receiver
    default = .91
    outputs = none
  []
  [n_elements]
    type = NumElements
    execute_on = 'timestep_end'
  []
[]

[VectorPostprocessors]
  [xfcn_vpp]
    type = VectorOfPostprocessors
    postprocessors = 'xfcn_pp x_pp'
    outputs = none
  []
[]

[Adaptivity]
  marker = x_moving
  max_h_level = 2
  [Markers]
    [x_moving]
      type = ReporterPointMarker
      x_coord_name = xfcn_vpp/xfcn_vpp
      y_coord_name = coords/y
      z_coord_name = coords/z
      inside = REFINE
      empty = COARSEN
    []
  []
[]

[Outputs]
  csv = true
[]
