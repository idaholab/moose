[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Adaptivity]
  marker = uniform
  # Enforces the mesh changing and not changing at different steps
  max_h_level = 2
  [Markers]
    [uniform]
      type = UniformMarker
      mark = REFINE
    []
  []
[]

[Problem]
  solve = false
[]

[GlobalParams]
  displacements = 'disp_x'
[]

[AuxVariables]
  [disp_x]
  []
[]

[AuxKernels]
  [disp_x_aux]
    type = ParsedAux
    expression = 'x * t'
    variable = disp_x
    use_displaced_mesh = false
    use_xyzt = true
  []
[]

[Postprocessors]
  [vol]
    type = VolumePostprocessor
    use_displaced_mesh = true
    execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  csv = true
[]
