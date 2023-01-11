[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
  uniform_refine = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [dot]
    type = TimeDerivative
    variable = u
  []
[]

[DiracKernels]
  [vpp_point_source]
    type = ReporterPointSource
    variable = u
    value_name = point_sample_source/u
    x_coord_name = point_sample_source/x
    y_coord_name = point_sample_source/y
    z_coord_name = point_sample_source/z
  []
[]

[VectorPostprocessors]
  [point_sample_source]
    type = PointValueSampler
    variable = u
    points = '0.2 0.8 0.0  0.2 0.2 0.0'
    sort_by = id
    execute_on = 'timestep_begin'
    outputs = none
  []
  [point_sample_out]
    type = PointValueSampler
    variable = u
    points = '0.2 0.8 0.0'
    sort_by = id
    execute_on = 'timestep_begin'
    contains_complete_history = true
    outputs = 'csv'
  []
[]

[Functions]
  [left_bc_fn]
    type = ParsedFunction
    expression = 1+5*y*y
  []
[]

[BCs]
  [left]
    type = FunctionNeumannBC
    variable = u
    boundary = left
    function = left_bc_fn
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 5
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
[]

[Outputs]
  csv = true
[]
