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

[Variables/u_adjoint]
  initial_condition = 0
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u_adjoint
  []
  [diff]
    type = MatDiffusion
    variable = u_adjoint
    diffusivity = D
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
    reverse_time_end = 1
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u_adjoint
    boundary = 'right top'
    value = 0
  []
[]

[Materials]
  [diffc]
    type = GenericFunctionMaterial
    prop_names = 'D'
    prop_values = 'diffc_fun'
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
                          0.1  10   10   0.1' # Reference solution
    outputs = none
  []
  [data]
    type = OptimizationData
  []
[]

[AuxVariables/u]
[]

[UserObjects]
  [load_u]
    type = AdjointSolutionUserObject
    mesh = forward_out.e
    system_variables = 'u'
    reverse_time_end = 1
    execute_on = 'timestep_begin'
  []
[]

[AuxKernels]
  [u_aux]
    type = SolutionAux
    variable = u
    solution = load_u
    direct = true
    execute_on = 'timestep_begin'
  []
[]

[VectorPostprocessors]
  [adjoint]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = u_adjoint
    forward_variable = u
    function = diffc_fun
    reverse_time_end = 1
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
