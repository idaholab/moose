[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[Variables/u]
  initial_condition = 0
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = D
  []
  [src]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'right top'
    value = 0
  []
[]

[Materials]
  [diffc]
    type = GenericFunctionMaterial
    prop_names = 'D'
    prop_values = 'diffc_fun'
    output_properties = 'D'
    outputs = 'exodus'
  []
[]

[Functions]
  [diffc_fun]
    type = NearestReporterCoordinatesFunction
    value_name = 'diffc_rep/D_vals'
    x_coord_name = 'diffc_rep/D_x_coord'
    y_coord_name = 'diffc_rep/D_y_coord'
  []
[]

[Reporters]
  [diffc_rep]
    type = ConstantReporter
    real_vector_names = 'D_x_coord D_y_coord D_vals'
    real_vector_values = '0.25 0.75 0.25 0.75;
                          0.25 0.25 0.75 0.75;
                          1  0.2   0.2   0.05' # Reference solution
    outputs = none
  []
  [data]
    type = OptimizationData
    variable = u
    measurement_points = '0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0
                          0.25 0.25 0  0.25 0.75 0  0.75 0.25 0  0.75 0.75 0'
    measurement_times = '0.1 0.1 0.1 0.1
                         0.2 0.2 0.2 0.2
                         0.3 0.3 0.3 0.3
                         0.4 0.4 0.4 0.4
                         0.5 0.5 0.5 0.5
                         0.6 0.6 0.6 0.6
                         0.7 0.7 0.7 0.7
                         0.8 0.8 0.8 0.8
                         0.9 0.9 0.9 0.9
                         1.0 1.0 1.0 1.0'
    measurement_values = '0 0 0 0
                          0 0 0 0
                          0 0 0 0
                          0 0 0 0
                          0 0 0 0
                          0 0 0 0
                          0 0 0 0
                          0 0 0 0
                          0 0 0 0
                          0 0 0 0'
  []
[]

[Postprocessors]
  [D1]
    type = PointValue
    variable = D
    point = '0.25 0.25 0'
  []
  [D2]
    type = PointValue
    variable = D
    point = '0.75 0.25 0'
  []
  [D3]
    type = PointValue
    variable = D
    point = '0.25 0.75 0'
  []
  [D4]
    type = PointValue
    variable = D
    point = '0.75 0.75 0'
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-12

  dt = 0.1
  num_steps = 10
[]

[Outputs]
  csv = true
  exodus = true
[]
