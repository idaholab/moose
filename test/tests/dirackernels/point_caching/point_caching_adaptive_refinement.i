[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
  uniform_refine = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  active = 'point_source'

  [./point_source]
    type = CachingPointSource
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Adaptivity]
  steps = 3
  marker = 'combo'

  [./Markers]
    [./combo]
      # In a real problem you would want to mark based on an error
      # indicator, but we want this test to run consistently in
      # parallel, so we just mark elements within a box for
      # refinement.  The boxes here are based on the 8x8
      # uniformly-refined initial grid.
      type = ComboMarker
      markers = 'box1 box2 box3'
    [../]

    [./box1]
      type = BoxMarker
      bottom_left = '0.125 0.625 0'
      top_right = '0.375 0.875 0'
      inside = refine
      outside = dont_mark
    [../]

    [./box2]
      type = BoxMarker
      bottom_left = '0.625 0.625 0'
      top_right = '0.875 0.875 0'
      inside = refine
      outside = dont_mark
    [../]

    [./box3]
      type = BoxMarker
      bottom_left = '0.625 0.125 0'
      top_right = '0.875 0.375 0'
      inside = refine
      outside = dont_mark
    [../]
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
