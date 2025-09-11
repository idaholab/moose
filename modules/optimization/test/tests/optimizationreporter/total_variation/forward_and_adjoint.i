# Forward and adjoint solve for parameter estimation
[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = FALSE
[]
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 15
    ny = 15
    xmin = -1.0
    xmax = 1.0
    ymin = -1.0
    ymax = 1.0
  []
[]

[Variables]
  [temperature]
  []
  [adjoint_temperature]
    solver_sys = adjoint
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = temperature
    diffusivity = thermal_conductivity
  []
  [heat_source]
    type = FunctorKernel
    variable = temperature
    functor = source_function
    functor_on_rhs = true
  []
[]

[BCs]
  [all_sides]
    type = ADDirichletBC
    variable = temperature
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Functions]
  [source_function]
    type = ParameterMeshFunction
    parameter_name = parameters/source
    exodus_mesh = 'initial_param_mesh_in.e'
  []
[]
[AuxVariables]
  [source]
    [AuxKernel]
      type = FunctionAux
      function = source_function
    []
  []
[]

[Materials]
  [thermal_conductivity]
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 1
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[DiracKernels]
  [misfit]
    type = ReporterPointSource
    variable = adjoint_temperature
    x_coord_name = measurement_data/measurement_xcoord
    y_coord_name = measurement_data/measurement_ycoord
    z_coord_name = measurement_data/measurement_zcoord
    value_name = measurement_data/misfit_values
  []
[]

[Reporters]
  [measurement_data]
    type = OptimizationData
    objective_name = objective_value
    variable = temperature
  []
  [parameters]
    type = ConstantReporter
    real_vector_names = 'source'
    real_vector_values = '0'
  []
[]

[VectorPostprocessors]
  [gradient]
    type = ElementOptimizationSourceFunctionInnerProduct
    variable = adjoint_temperature
    function = source_function
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Outputs]
  console = false
  exodus = true
[]
