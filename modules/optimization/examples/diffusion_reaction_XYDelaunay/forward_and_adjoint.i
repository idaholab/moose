[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 25
    ny = 25
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[Variables/u]

[]

[Reporters]
  [params]
    type = ConstantReporter
    real_vector_names = 'reaction_rate'
    real_vector_values = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0' # Dummy
    outputs = nones
  []
  [data]
    type = OptimizationData
    variable = u
    objective_name = objective_value
    measurement_file = forward_exact_csv_sample_0011.csv
    file_xcoord = measurement_xcoord
    file_ycoord = measurement_ycoord
    file_zcoord = measurement_zcoord
    file_time = measurement_time
    file_value = simulation_values
    outputs = none
  []
[]

[Functions]
  [rxn_func]
    type = ParameterMeshFunction
    exodus_mesh = parameter_mesh_out.e
    parameter_name = params/reaction_rate
  []
[]

[Materials]
  [ad_dc_prop]
    type = ADParsedMaterial
    expression = '1 + u'
    coupled_variables = 'u'
    property_name = dc_prop
  []
  [ad_rxn_prop]
    type = ADGenericFunctionMaterial
    prop_values = 'rxn_func'
    prop_names = rxn_prop
  []
  #ADMatReaction includes a negative sign in residual evaluation, so we need to
  #reverse this with a negative reaction rate. However, we wanted the parameter
  #to remain positive, which is why there is one object to evaluate function
  #and another to flip it's sign for the kernel
  [ad_neg_rxn_prop]
    type = ADParsedMaterial
    expression = '-rxn_prop'
    material_property_names = 'rxn_prop'
    property_name = 'neg_rxn_prop'
  []
[]

[Kernels]
  [udot]
    type = ADTimeDerivative
    variable = u
  []
  [diff]
    type = ADMatDiffusion
    variable = u
    diffusivity = dc_prop
  []
  [reaction]
    type = ADMatReaction
    variable = u
    reaction_rate = neg_rxn_prop
  []
  [src]
    type = ADBodyForce
    variable = u
    value = 1
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'left bottom'
    value = 0
  []
[]

[Executioner]
  type = TransientAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 0.1
  end_time = 1

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
  skip_nl_system_check = true
[]

[Variables]
  [u_adjoint]
    initial_condition = 0
    solver_sys = adjoint

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
    type = ElementOptimizationReactionFunctionInnerProduct
    variable = u_adjoint
    forward_variable = u
    function = rxn_func
    execute_on = ADJOINT_TIMESTEP_END
    outputs = none
  []
[]

[AuxVariables]
  [reaction_rate]
  []
[]

[AuxKernels]
  [reaction_rate_aux]
    type = FunctionAux
    variable = reaction_rate
    function = rxn_func
    execute_on = TIMESTEP_END
  []
[]

[Postprocessors]
  [u1]
    type = PointValue
    variable = u
    point = '0.25 0.25 0'
  []
  [u2]
    type = PointValue
    variable = u
    point = '0.75 0.75 0'
  []
  [u3]
    type = PointValue
    variable = u
    point = '1 1 0'
  []
[]

[Outputs]
  exodus = true
  console = false
  csv = true
[]
