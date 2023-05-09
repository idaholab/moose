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
  type = TransientAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-12
  l_tol = 1e-12

  dt = 0.1
  num_steps = 10
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [u_adjoint]
    initial_condition = 0
    nl_sys = adjoint
    outputs = none
  []
[]

[DiracKernels]
  [misfit]
    type = ReporterTimePointSource
    variable = u_adjoint
    value_name = data/misfit_values
    x_coord_name = data/measurement_xcoord
    y_coord_name = data/measurement_ycoord
    z_coord_name = data/measurement_zcoord
    time_name = data/measurement_time
  []
[]

[VectorPostprocessors]
  [adjoint]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = u_adjoint
    forward_variable = u
    function = diffc_fun
    execute_on = ADJOINT_TIMESTEP_END
    outputs = none
  []
[]

[Outputs]
  # The default exodus object executes only during the forward system solve,
  # so the adjoint variable in the resulting file will show only 0.
  # Unfortunately, there is no way to output the adjoint variable with Exodus.
  exodus = true
  console = false
[]
