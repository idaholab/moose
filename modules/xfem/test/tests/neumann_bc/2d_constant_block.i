[XFEM]
  output_cut_plane = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 9
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = 'gmg'
    block_id = 0
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 1
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
[]

[UserObjects]
  [line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '0.0 0.5 1 0.5'
  []
[]

[Variables]
  [temp]
    initial_condition = 200
  []
[]

[Kernels]
  [heat_dt]
    type = TimeDerivative
    variable = temp
  []
  [heat_conduction]
    type = MatDiffusion
    variable = temp
    diffusivity = 1
  []
  [heat]
    type = BodyForce
    variable = temp
    value = 0
  []
[]

[DiracKernels]
  [right]
    type = XFEMNeumannBC
    variable = temp
    geometric_cut_userobject = line_seg_cut_uo
    value = -1
    block = 1
  []
[]

[Executioner]
  type = Transient

  num_steps = 10
  dt = 1e1

  nl_abs_tol = 1e-12

  automatic_scaling = true
[]

[Outputs]
  exodus = true
[]
