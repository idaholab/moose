[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Functions]
  [parameter_mesh]
    type = ParameterMeshFunction
    exodus_mesh = create_mesh_out.e
    parameter_name = param_vec/params
    time_name = param_vec/times
  []
[]

[Reporters]
  [param_vec]
    type = ConstantReporter
    real_vector_names = 'times params'
    real_vector_values = '0 2 5;
                          0 0 0.0000 0 0 0 0 0 0
                          0 0 0.1250 0 0 0 0 0 0
                          0 0 0.3125 0 0 0 0 0 0'
  []
[]

[AuxVariables]
  [parameter]
    family = MONOMIAL
    order = CONSTANT
  []
  [grad_parameter]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
  [parameter_gradient]
    components = 27
  []
[]

[AuxKernels]
  [parameter_aux]
    type = FunctionAux
    function = parameter_mesh
    variable = parameter
    execute_on = 'initial timestep_end'
  []
  [grad_parameter_aux]
    type = FunctorElementalGradientAux
    functor = parameter_mesh
    variable = grad_parameter
    execute_on = 'initial timestep_end'
  []
  [parameter_gradient_aux]
    type = OptimizationFunctionAuxTest
    function = parameter_mesh
    variable = parameter_gradient
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
[]

[Problem]
  solve = false
[]
