[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [u]
  []
  [u_adjoint]
    nl_sys = adjoint
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [src]
    type = BodyForce
    variable = u
    function = src_func
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'bottom left'
    value = 0
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = u_adjoint
    x_coord_name = measure_data/measurement_xcoord
    y_coord_name = measure_data/measurement_ycoord
    z_coord_name = measure_data/measurement_zcoord
    value_name = measure_data/misfit_values
  []
[]

[Functions]
  [src_func]
    type = ParameterMeshFunction
    exodus_mesh = parameter_mesh_in.e
    parameter_name = src_rep/vals
  []
[]

[Reporters]
  [src_rep]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0'
  []
  [measure_data]
    type = OptimizationData
    variable = u
  []
[]

[VectorPostprocessors]
  [gradient_vpp]
    type = ElementOptimizationSourceFunctionInnerProduct
    variable = u_adjoint
    function = src_func
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-12
  l_tol = 1e-12
[]

[AuxVariables]
  [source]
  []
[]

[AuxKernels]
  [source_aux]
    type = FunctionAux
    variable = source
    function = src_func
  []
[]

[Outputs]
  exodus = true
  console = true
  execute_on = timestep_end
[]
