[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
  []
  [box1]
    type = SubdomainBoundingBoxGenerator
    input = box
    block_id = 1
    bottom_left = '0 0.5 0'
    top_right = '1 1 1'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
    function = '(x-0.5) * (x-0.5) * (x-0.5)'
  []
[]

[BCs]
  [archor_x]
    type = DirichletBC
    boundary = 'bottom'
    variable = u
    value = 0
  []
[]

[Adaptivity]
  marker = errorfrac
  max_h_level = 2
  [Indicators]
    [error]
      type = GradientJumpIndicator
      variable = u
      block = '0'
    []
  []
  [Markers]
    [errorfrac]
      type = ErrorFractionMarker
      refine = 0.5
      indicator = error
      block = '0'
    []
  []
[]

[Executioner]
  type = Transient

  end_time = 200
  dt = 100

  solve_type = 'PJFNK'

  nl_abs_tol = 1E-5
  nl_max_its = 400

  l_tol = 1E-8
  l_max_its = 200
[]

[Outputs]
  exodus = true
[]
