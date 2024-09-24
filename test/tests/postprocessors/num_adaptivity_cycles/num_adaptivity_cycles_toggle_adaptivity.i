[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./force]
    type = ParsedFunction
    expression = t
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = BodyForce
    variable = u
    function = force
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 1

  solve_type = 'PJFNK'

[]

[Adaptivity]
  cycles_per_step = 1
  marker = box
  max_h_level = 2
  initial_steps = 4
  initial_marker = initial_box

  # backwards compatibility for exodiffs after #25067
  project_initial_marker = true

  [./Markers]
    [./box]
      bottom_left = '0.3 0.3 0'
      inside = refine
      top_right = '0.6 0.6 0'
      outside = dont_mark
      type = BoxMarker
    [../]
    [./initial_box]
      type = BoxMarker
      bottom_left = '0.8 0.1 0'
      top_right = '0.9 0.2 0'
      inside = refine
      outside = dont_mark
    [../]
  [../]
[]

[UserObjects]
  [./toggle_adaptivity]
    type = ToggleMeshAdaptivity
    mesh_adaptivity = 'off'
  [../]
[]

[Postprocessors]
  [./adaptivity_cycles]
    type = NumAdaptivityCycles
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  csv = true
[]
