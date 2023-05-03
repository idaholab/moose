# Steady state Heat conduction in a 2D domain with two diffusivities
# The domain is -4 <= x <= 4 and -4 <= y <= 4
# The top-half of the domain (y > 0) has high diffusivity
# The bottom-half of the domain (y < 0) has low diffusivity

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
    xmin = -4
    xmax = 4
    ymin = -4
    ymax = 4
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [temperature]
  []
  [temperature_adjoint]
    nl_sys = adjoint
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = temperature
    diffusivity = diffusivity
  []
  [heat_source]
    type = BodyForce
    value = 100
    variable = temperature
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
[]

[Functions]
  [diffusivity_function]
    type = NearestReporterCoordinatesFunction
    x_coord_name = data/coordx
    y_coord_name = data/coordy
    value_name = data/diffusivity
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = diffusivity
    prop_values = diffusivity_function
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = temperature_adjoint
    x_coord_name = measure_data/measurement_xcoord
    y_coord_name = measure_data/measurement_ycoord
    z_coord_name = measure_data/measurement_zcoord
    value_name = measure_data/misfit_values
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
  []
  [data]
    type = ConstantReporter
    real_vector_names = 'coordx coordy diffusivity'
    real_vector_values = '0 0; -2 2; 5 10'
  []
[]

[VectorPostprocessors]
  [gradvec]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = temperature_adjoint
    forward_variable = temperature
    function = diffusivity_function
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_forced_its = 1
  line_search = none
  nl_abs_tol = 1e-8
[]

[Outputs]
  console = false
[]
